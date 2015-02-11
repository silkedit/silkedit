#pragma once

#include <msgpack/rpc/protocol.h>
#include <QObject>
#include <QProcess>
#include <QLocalSocket>
#include <QLocalServer>

#include "macros.h"
#include "Singleton.h"

class PluginService : public QObject, public Singleton<PluginService> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(PluginService)

 public:
  ~PluginService();

  void init();

 private:
  friend class Singleton<PluginService>;
  PluginService();

  QProcess* m_pluginProcess;
  QLocalSocket* m_socket;
  QLocalServer* m_server;

 private:
  template <typename Result, typename Error>
  void call(Result& res, Error& err, msgpack::rpc::msgid_t id);

 private slots:
  void readStdout();
  void pluginRunnerConnected();
  void error(QProcess::ProcessError error);
  void readRequest();
  void displayError(QLocalSocket::LocalSocketError);
};
