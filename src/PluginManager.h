#pragma once

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

#include "macros.h"
#include "msgpackHelper.h"
#include "stlSpecialization.h"
#include "Singleton.h"
#include "IContext.h"
#include "IKeyEventFilter.h"
#include "CommandArgument.h"
#include "CommandManager.h"

class QKeyEvent;

class ResponseResult : public QObject {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(ResponseResult)

 public:
  ResponseResult() = default;
  ~ResponseResult() { qDebug("~ResponseResult"); }

  bool isReady() { return m_isReady; }
  bool isSuccess() { return m_isSuccess; }
  msgpack::object result() { return m_result.object; }

  void setResult(const object_with_zone& obj);
  void setError(const object_with_zone& obj);

signals:
  void ready();

 private:
  bool m_isReady;
  bool m_isSuccess;
  object_with_zone m_result;
};

class PluginManager : public QObject, public Singleton<PluginManager>, public IKeyEventFilter {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(PluginManager)

 public:
  ~PluginManager();

  void init();

  void callExternalCommand(const QString& cmd, CommandArgument args);
  bool askExternalContext(const QString& name, Operator op, const QString& value);

  // IKeyEventFilter interface
  bool keyEventFilter(QKeyEvent* event) override;

  template <typename Parameter, typename Result>
  Result sendRequest(const std::string& method,
                     const Parameter& params,
                     msgpack::type::object_type type) {
    qDebug("sendRequest. method: %s", method.c_str());
    msgpack::sbuffer sbuf;
    msgpack::rpc::msg_request<std::string, Parameter> request;
    request.method = method;
    msgpack::rpc::msgid_t msgId = s_msgId++;
    request.msgid = msgId;
    request.param = params;
    msgpack::pack(sbuf, request);

    m_socket->write(sbuf.data(), sbuf.size());

    QEventLoop loop;
    ResponseResult result;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    s_eventLoopMap.insert(std::make_pair(msgId, &result));
    connect(&result, &ResponseResult::ready, &loop, &QEventLoop::quit);

    timer.start(TIMEOUT_IN_MS);
    // start a local event loop to wait until plugin runner returns response or timeout occurs
    loop.exec();

    if (result.isReady()) {
      if (result.isSuccess()) {
        if (result.result().type == type) {
          return result.result().as<Result>();
        } else {
          throw std::runtime_error("unexpected result type");
        }
      } else {
        std::ostringstream out;
        out << "error." << result.result().as<std::string>();
        throw std::runtime_error(out.str());
      }
    } else {
      throw std::runtime_error("timeout for waiting the result");
    }
  }

  template <typename Result, typename Error>
  void sendResponse(const Result& res, const Error& err, msgpack::rpc::msgid_t id) {
    msgpack::sbuffer sbuf;
    msgpack::rpc::msg_response<Result, Error> response;
    response.msgid = id;
    response.error = err;
    response.result = res;

    msgpack::pack(sbuf, response);

    m_socket->write(sbuf.data(), sbuf.size());
  }

signals:
  void readyRequest(const QString& method, msgpack::rpc::msgid_t msgId, const object_with_zone& args);
  void readyNotify(const QString& method, const object_with_zone& args);

 private:
  static std::unordered_map<QString, std::function<void(const QString&, const msgpack::object&)>>
      s_notifyFunctions;
  static std::unordered_map<
      QString,
      std::function<void(msgpack::rpc::msgid_t, const QString&, const msgpack::object&)>>
      s_requestFunctions;
  static msgpack::rpc::msgid_t s_msgId;
  static std::unordered_map<msgpack::rpc::msgid_t, ResponseResult*> s_eventLoopMap;

  friend class Singleton<PluginManager>;
  PluginManager();

  QProcess* m_pluginProcess;
  QLocalSocket* m_socket;
  QLocalServer* m_server;

 private:
  static constexpr auto TIMEOUT_IN_MS = 1000;

  void readStdout();
  void readStderr();
  void pluginRunnerConnected();
  void error(QProcess::ProcessError error);
  void readRequest();
  void displayError(QLocalSocket::LocalSocketError);
  std::tuple<bool, std::string, CommandArgument> cmdEventFilter(const std::string& name,
                                                                const CommandArgument& arg);
  void callRequestFunc(const QString& method, msgpack::rpc::msgid_t msgId, object_with_zone args);
  void callNotifyFunc(const QString& method, object_with_zone args);
};
