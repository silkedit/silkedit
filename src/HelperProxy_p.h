#pragma once

#include "HelperProxy.h"
#include "core/IKeyEventFilter.h"

class HelperProxyPrivate : public QObject, public core::IKeyEventFilter {
  Q_OBJECT
 public:
  static std::unordered_map<QString, std::function<void(const QString&, const msgpack::object&)>>
      s_notifyFunctions;
  static std::unordered_map<
      QString,
      std::function<void(msgpack::rpc::msgid_t, const QString&, const msgpack::object&)>>
      s_requestFunctions;

  HelperProxy* q;
  std::unique_ptr<QProcess> m_helperProcess;
  QLocalServer* m_server;

  explicit HelperProxyPrivate(HelperProxy* q_ptr);
  ~HelperProxyPrivate();

  void init();

  // IKeyEventFilter interface
  bool keyEventFilter(QKeyEvent* event) override;
  void readStdout();
  void readStderr();
  void helperConnected();
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
