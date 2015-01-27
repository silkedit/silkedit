#pragma once

#include <QObject>
#include <QProcess>
#include <QLocalSocket>

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

 private slots:
  void readStdout();
  void started();
  void error(QProcess::ProcessError error);
  void readRequest();
  void displayError(QLocalSocket::LocalSocketError);
};
