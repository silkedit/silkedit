#pragma once

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
  void unpack(const char* buffer, std::size_t len);

 private slots:
  void readStdout();
  void pluginRunnerConnected();
  void error(QProcess::ProcessError error);
  void readRequest();
  void displayError(QLocalSocket::LocalSocketError);
};
