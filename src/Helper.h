#pragma once

#include <boost/optional.hpp>
#include <functional>
#include <string>
#include <unordered_map>
#include <tuple>
#include <sstream>
#include <msgpack.hpp>
#include <msgpack/rpc/protocol.h>
#include <QObject>
#include <QProcess>
#include <QLocalSocket>
#include <QLocalServer>
#include <QEventLoop>
#include <QTimer>

#include "CommandArgument.h"
#include "core/macros.h"
#include "core/msgpackHelper.h"
#include "core/stlSpecialization.h"
#include "core/Singleton.h"
#include "core/Icondition.h"
#include "qvariant_adapter.h"

class HelperPrivate;

class ResponseResult : public QObject {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(ResponseResult)

 public:
  ResponseResult() = default;
  ~ResponseResult() = default;

  bool isReady() { return m_isReady; }
  bool isSuccess() { return m_isSuccess; }
  msgpack::object result() { return m_result->object; }

  void setResult(std::unique_ptr<object_with_zone> obj);
  void setError(std::unique_ptr<object_with_zone> obj);

 signals:
  void ready();

 private:
  bool m_isReady;
  bool m_isSuccess;
  std::unique_ptr<object_with_zone> m_result;
};

class GetRequestResponse : public QObject {
  Q_OBJECT
  DISABLE_COPY(GetRequestResponse)

 public:
  GetRequestResponse() = default;
  ~GetRequestResponse() { qDebug("~GetRequestResponse"); }
  DEFAULT_MOVE(GetRequestResponse)

 signals:
  void onSucceeded(const QString& body);
  void onFailed(const QString& error);
};

/**
 * @brief Proxy object to interact with silkedit_helper Node.js process
 */
class Helper : public QObject, public core::Singleton<Helper> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(Helper)

 public:
  ~Helper();

  void init();

  void sendFocusChangedEvent(const QString& viewType);
  void sendCommandEvent(const QString& command, const CommandArgument& args);
  void callExternalCommand(const QString& cmd, const CommandArgument& args);
  bool askExternalCondition(const QString& name, core::Operator op, const QString& value);
  QString translate(const QString& key, const QString& defaultValue);
  void loadPackage(const QString& pkgName);
  bool removePackage(const QString& pkgName);
  GetRequestResponse* sendGetRequest(const QString& url, int timeoutInMs);
  void reloadKeymaps();

  void sendNotification(const std::string& method) {
    if (m_isStopped) {
      return;
    }

    if (!m_socket) {
      qWarning("socket has not been initialized yet");
      return;
    }

    msgpack::sbuffer sbuf;
    msgpack::rpc::msg_notify_no_param<std::string> notify;
    notify.method = method;
    msgpack::pack(sbuf, notify);

    m_socket->write(sbuf.data(), sbuf.size());
  }

  template <typename Parameter>
  void sendNotification(const std::string& method, const Parameter& params) {
    if (m_isStopped) {
      return;
    }

    if (!m_socket) {
      qWarning("socket has not been initialized yet");
      return;
    }

    msgpack::sbuffer sbuf;
    msgpack::rpc::msg_notify<std::string, Parameter> notify;
    notify.method = method;
    notify.param = params;
    msgpack::pack(sbuf, notify);

    m_socket->write(sbuf.data(), sbuf.size());
  }

  /**
   * @brief Send a request via msgpack rpc.
   * Throws an exception if the response type is not an expected type.
   * @param method
   * @param params
   * @param type
   * @return return a response as the expected type.
   */
  template <typename Parameter, typename Result>
  Result sendRequest(const std::string& method,
                     const Parameter& params,
                     msgpack::type::object_type type,
                     int timeoutInMs = TIMEOUT_IN_MS) {
    std::unique_ptr<ResponseResult> result =
        sendRequestInternal<Parameter, Result>(method, params, timeoutInMs);
    if (result->result().type == type) {
      return result->result().as<Result>();
    } else {
      throw std::runtime_error("unexpected result type");
    }
  }

  /**
   * @brief Send a requesta asynchronously via msgpack rpc.
   */
  template <typename Parameter, typename Result>
  void sendRequestAsync(const std::string& method,
                        const Parameter& params,
                        msgpack::type::object_type type,
                        std::function<void(const Result&)> onSuccess,
                        std::function<void(const QString&)> onFailure,
                        int timeoutInMs = TIMEOUT_IN_MS) {
    msgpack::rpc::msgid_t msgId = sendRequestInternalAsync<Parameter>(method, params);
    assert(s_eventLoopMap.count(msgId) != 0);
    ResponseResult* result = s_eventLoopMap[msgId];
    assert(result);
    QTimer::singleShot(timeoutInMs, result, [=] {
      result->deleteLater();
      onFailure("timeout");
    });
    connect(result, &ResponseResult::ready, [=] {
      result->deleteLater();
      if (result->result().type == type) {
        onSuccess(result->result().as<Result>());
      } else {
        onFailure("unexpected result type");
      }
    });
  }

  /**
   * @brief Send a request via msgpack rpc.
   * Returns none if the response type is msgpack::Nil
   * Throws an exception if the response type is not null and an expected type.
   * @param method
   * @param params
   * @param type
   * @return return a response as the specified type.
   */
  template <typename Parameter, typename Result>
  boost::optional<Result> sendRequestOption(const std::string& method,
                                            const Parameter& params,
                                            msgpack::type::object_type type,
                                            int timeoutInMs = TIMEOUT_IN_MS) {
    std::unique_ptr<ResponseResult> result =
        sendRequestInternal<Parameter, Result>(method, params, timeoutInMs);
    if (result->result().type == msgpack::type::NIL) {
      return boost::none;
    } else if (result->result().type == type) {
      return result->result().as<Result>();
    } else {
      throw std::runtime_error("unexpected result type");
    }
  }

  template <typename Result, typename Error>
  void sendResponse(const Result& res, const Error& err, msgpack::rpc::msgid_t id) {
    if (m_isStopped) {
      return;
    }

    msgpack::sbuffer sbuf;
    msgpack::rpc::msg_response<Result, Error> response;
    response.msgid = id;
    response.error = err;
    response.result = res;

    msgpack::pack(sbuf, response);

    m_socket->write(sbuf.data(), sbuf.size());
  }

 private:
  static msgpack::rpc::msgid_t s_msgId;
  static std::unordered_map<msgpack::rpc::msgid_t, ResponseResult*> s_eventLoopMap;
  static const int TIMEOUT_IN_MS = 1000;

  std::unique_ptr<HelperPrivate> d;
  bool m_isStopped;
  QLocalSocket* m_socket;

  friend class HelperPrivate;
  friend class core::Singleton<Helper>;
  Helper();

  void callNotifyFunc(const QString& method, const msgpack::object& obj);
  void callRequestFunc(msgpack::rpc::msgid_t msgId,
                       const QString& method,
                       const msgpack::object& obj);

  // Send a request via msgpack rpc.
  template <typename Parameter, typename Result>
  std::unique_ptr<ResponseResult> sendRequestInternal(const std::string& method,
                                                      const Parameter& params,
                                                      int timeoutInMs) {
    if (m_isStopped) {
      throw std::runtime_error("silkedit_helper is not running");
    }

    //    qDebug("sendRequest. method: %s", method.c_str());
    msgpack::sbuffer sbuf;
    msgpack::rpc::msg_request<std::string, Parameter> request;
    request.method = method;
    // todo: check overlow
    msgpack::rpc::msgid_t msgId = s_msgId++;
    request.msgid = msgId;
    request.param = params;
    msgpack::pack(sbuf, request);

    if (!m_socket) {
      throw std::runtime_error("socket has not been initialized yet");
    }
    m_socket->write(sbuf.data(), sbuf.size());

    QEventLoop loop;
    std::unique_ptr<ResponseResult> result(new ResponseResult());
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    s_eventLoopMap.insert(std::make_pair(msgId, result.get()));
    connect(result.get(), &ResponseResult::ready, &loop, &QEventLoop::quit);

    if (timeoutInMs > 0) {
      timer.start(timeoutInMs);
    }
    // start a local event loop to wait until helper returns response or timeout occurs
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    if (result->isReady()) {
      if (result->isSuccess()) {
        return std::move(result);
      } else {
        std::string errMsg = result->result().as<std::string>();
        throw std::runtime_error(errMsg);
      }
    } else {
      throw std::runtime_error("timeout for waiting the result");
    }
  }

  // Send a request asynchronously via msgpack rpc.
  template <typename Parameter>
  msgpack::rpc::msgid_t sendRequestInternalAsync(const std::string& method,
                                                 const Parameter& params) {
    if (m_isStopped) {
      throw std::runtime_error("silkedit_helper is not running");
    }

    msgpack::sbuffer sbuf;
    msgpack::rpc::msg_request<std::string, Parameter> request;
    request.method = method;
    // todo: Check overflow
    msgpack::rpc::msgid_t msgId = s_msgId++;
    request.msgid = msgId;
    request.param = params;
    msgpack::pack(sbuf, request);

    if (!m_socket) {
      throw std::runtime_error("socket has not been initialized yet");
    }
    m_socket->write(sbuf.data(), sbuf.size());

    ResponseResult* result = new ResponseResult();

    s_eventLoopMap.insert(std::make_pair(msgId, result));
    connect(result, &ResponseResult::destroyed, [=] { s_eventLoopMap.erase(msgId); });

    return msgId;
  }

  std::tuple<bool, std::string, CommandArgument> cmdEventFilter(const std::string& name,
                                                                const CommandArgument& arg);
};
