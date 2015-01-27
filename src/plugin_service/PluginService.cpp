#include <QProcess>
#include <QDebug>

#include "PluginService.h"
#include "SilkApp.h"
#include "Constants.h"

PluginService::~PluginService() {
  qDebug("~PluginService");
  // todo: stop plugin server.
}

void PluginService::init() {
  Q_ASSERT(!m_pluginProcess);

  m_pluginProcess = new QProcess(this);
  connect(m_pluginProcess, &QProcess::started, this, &PluginService::started);
  connect(m_pluginProcess,
          SIGNAL(error(QProcess::ProcessError)),
          this,
          SLOT(error(QProcess::ProcessError)));
  connect(m_pluginProcess, &QProcess::readyReadStandardOutput, this, &PluginService::readStdout);
  qDebug("server path: %s", qPrintable(Constants::pluginServerPath()));
  qDebug() << "args:" << Constants::pluginServerArgs();
  m_pluginProcess->start(Constants::pluginServerPath(), Constants::pluginServerArgs());
}

PluginService::PluginService() : m_pluginProcess(nullptr), m_socket(nullptr) {
}

void PluginService::readStdout() {
  qDebug() << "readyOut";
  QProcess* p = (QProcess*)sender();
  QByteArray buf = p->readAllStandardOutput();

  qDebug() << buf;
}

void PluginService::started() {
  qDebug() << "Server Started";
  m_socket = new QLocalSocket(this);
  connect(m_socket, &QLocalSocket::readyRead, this, &PluginService::readRequest);
  connect(m_socket,
          SIGNAL(error(QLocalSocket::LocalSocketError)),
          this,
          SLOT(displayError(QLocalSocket::LocalSocketError)));
  m_socket->abort();
  m_socket->connectToServer(Constants::pluginServerSocketPath());
}

void PluginService::error(QProcess::ProcessError error) {
  qDebug() << "Error: " << error;
}

void PluginService::readRequest() {
  QTextStream in(m_socket);
  qDebug() << qPrintable(in.readAll());
}

void PluginService::displayError(QLocalSocket::LocalSocketError socketError) {
  switch (socketError) {
    case QLocalSocket::ServerNotFoundError:
      qWarning("The host was not found.");
      break;
    case QLocalSocket::ConnectionRefusedError:
      qWarning("The connection was refused by the peer.");
      break;
    case QLocalSocket::PeerClosedError:
      break;
    default:
      qWarning("The following error occurred: %s.", qPrintable(m_socket->errorString()));
      break;
  }
}
