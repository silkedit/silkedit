#pragma once

#include <boost/optional.hpp>
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

#include "core/macros.h"
#include "core/msgpackHelper.h"
#include "stlSpecialization.h"
#include "core/Singleton.h"
#include "core/IContext.h"
#include "core/IKeyEventFilter.h"
#include "CommandArgument.h"
#include "PluginManager_p.h"

class QKeyEvent;

class PluginManager : public core::Singleton<PluginManager>, public core::IKeyEventFilter {
  DISABLE_COPY_AND_MOVE(PluginManager)

 public:
  ~PluginManager();

  void init();

  void sendFocusChangedEvent(const QString& viewType);
  void sendCommandEvent(const QString& command, const CommandArgument& args);
  void callExternalCommand(const QString& cmd, const CommandArgument& args);
  bool askExternalContext(const QString& name, core::Operator op, const QString& value);
  QString translate(const std::string& key, const QString& defaultValue);

  // IKeyEventFilter interface
  bool keyEventFilter(QKeyEvent* event) override;

  template <typename Parameter>
  void sendNotification(const std::string& method, const Parameter& params) {
    d->sendNotification<Parameter>(method, params);
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
                     msgpack::type::object_type type) {
    return d->sendRequest<Parameter, Result>(method, params, type);
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
                                            msgpack::type::object_type type) {
    return d->sendRequestOption<Parameter, Result>(method, params, type);
  }

  template <typename Result, typename Error>
  void sendResponse(const Result& res, const Error& err, msgpack::rpc::msgid_t id) {
    d->sendResponse<Result, Error>(res, err, id);
  }

 private:
  std::unique_ptr<PluginManagerPrivate> d;

  friend class PluginManagerPrivate;
  friend class core::Singleton<PluginManager>;
  PluginManager();

  std::tuple<bool, std::string, CommandArgument> cmdEventFilter(const std::string& name,
                                                                const CommandArgument& arg);
  void callRequestFunc(const QString& method,
                       msgpack::rpc::msgid_t msgId,
                       const msgpack::object& obj);
  void callNotifyFunc(const QString& method, const msgpack::object& obj);
};
