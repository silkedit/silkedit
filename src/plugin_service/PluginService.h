#pragma once

#include <msgpack/rpc/protocol.h>
#include <QObject>
#include <QProcess>
#include <QLocalSocket>
#include <QLocalServer>

#include "macros.h"
#include "Singleton.h"
#include "ICommand.h"

class PluginService : public QObject, public Singleton<PluginService> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(PluginService)

 public:
  ~PluginService();

  void init();
  void callExternalCommand(const QString& cmd);

 private:
  friend class Singleton<PluginService>;
  PluginService();

  QProcess* m_pluginProcess;
  QLocalSocket* m_socket;
  QLocalServer* m_server;

 private:
  template <typename Result, typename Error>
  void sendResponse(const Result& res, const Error& err, msgpack::rpc::msgid_t id);

 private slots:
  void readStdout();
  void pluginRunnerConnected();
  void error(QProcess::ProcessError error);
  void readRequest();
  void displayError(QLocalSocket::LocalSocketError);
};

class PluginCommand : public ICommand {
 public:
  PluginCommand(const QString& name);
  ~PluginCommand() = default;
  DEFAULT_COPY_AND_MOVE(PluginCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
