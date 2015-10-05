#pragma once

#include "PluginManager.h"
#include "core/IKeyEventFilter.h"

class PluginManagerPrivate : public QObject, public core::IKeyEventFilter {
  Q_OBJECT
 public:
  static std::unordered_map<QString, std::function<void(const QString&, const msgpack::object&)>>
      s_notifyFunctions;
  static std::unordered_map<
      QString,
      std::function<void(msgpack::rpc::msgid_t, const QString&, const msgpack::object&)>>
      s_requestFunctions;

  PluginManager* q;
  std::unique_ptr<QProcess> m_pluginProcess;
  QLocalServer* m_server;

  explicit PluginManagerPrivate(PluginManager* q_ptr);
  ~PluginManagerPrivate();

  void init();

  void sendFocusChangedEvent(const QString& viewType);
  void sendCommandEvent(const QString& command, const CommandArgument& args);
  void callExternalCommand(const QString& cmd, const CommandArgument& args);
  bool askExternalContext(const QString& name, core::Operator op, const QString& value);
  QString translate(const std::string& key, const QString& defaultValue);

  // IKeyEventFilter interface
  bool keyEventFilter(QKeyEvent* event) override;
  void readStdout();
  void readStderr();
  void pluginRunnerConnected();
  void onFinished(int exitCode);
  void readRequest();
  void displayError(QLocalSocket::LocalSocketError);
  std::tuple<bool, std::string, CommandArgument> cmdEventFilter(const std::string& name,
                                                                const CommandArgument& arg);
  void callRequestFunc(const QString& method,
                       msgpack::rpc::msgid_t msgId,
                       const msgpack::object& obj);
  void callNotifyFunc(const QString& method, const msgpack::object& obj);

  void sendError(const std::string& err, msgpack::rpc::msgid_t id);

  void startPluginRunnerProcess();
};
