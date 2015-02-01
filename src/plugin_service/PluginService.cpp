#include <msgpack.hpp>
#include <QProcess>
#include <QDebug>
#include <QFile>

#include "PluginService.h"
#include "SilkApp.h"
#include "Constants.h"

PluginService::~PluginService() {
  qDebug("~PluginService");
  m_socket->disconnectFromServer();
  m_socket->close();
  m_pluginProcess->terminate();
}

void PluginService::init() {
  Q_ASSERT(!m_pluginProcess);

  m_server = new QLocalServer(this);
  QFile socketFile(Constants::pluginServerSocketPath());
  if (socketFile.exists()) {
    socketFile.remove();
  }

  if (!m_server->listen(Constants::pluginServerSocketPath())) {
    qCritical("Unable to start the server: %s", qPrintable(m_server->errorString()));
    return;
  }

  connect(m_server, &QLocalServer::newConnection, this, &PluginService::pluginRunnerConnected);

  m_pluginProcess = new QProcess(this);
  connect(m_pluginProcess,
          SIGNAL(error(QProcess::ProcessError)),
          this,
          SLOT(error(QProcess::ProcessError)));
  connect(m_pluginProcess, &QProcess::readyReadStandardOutput, this, &PluginService::readStdout);
  qDebug("plugin runner: %s", qPrintable(Constants::pluginRunnerPath()));
  qDebug() << "args:" << Constants::pluginRunnerArgs();
  m_pluginProcess->start(Constants::pluginRunnerPath(), Constants::pluginRunnerArgs());
}

PluginService::PluginService() : m_pluginProcess(nullptr), m_socket(nullptr), m_server(nullptr) {
}

void PluginService::readStdout() {
  qDebug() << "readyOut";
  QProcess* p = (QProcess*)sender();
  QByteArray buf = p->readAllStandardOutput();

  qDebug() << buf;
}

void PluginService::pluginRunnerConnected() {
  qDebug() << "new Plugin runner connected";

  m_socket = m_server->nextPendingConnection();
  connect(m_socket, SIGNAL(disconnected()), m_socket, SLOT(deleteLater()));
  connect(m_socket, &QLocalSocket::readyRead, this, &PluginService::readRequest);
  connect(m_socket,
          SIGNAL(error(QLocalSocket::LocalSocketError)),
          this,
          SLOT(displayError(QLocalSocket::LocalSocketError)));
}

void PluginService::error(QProcess::ProcessError error) {
  qDebug() << "Error: " << error;
}

void PluginService::readRequest() {
  qDebug("readRequest");

  // The size may decided by receive performance, transmit layer's protocol and so on.
  std::size_t const try_read_size = 100;

  msgpack::unpacker unp;

  // Message receive loop
  while (true) {
    unp.reserve_buffer(try_read_size);
    // unp has at least try_read_size buffer on this point.

    // input is a kind of I/O library object.
    // read message to msgpack::unpacker's internal buffer directly.
    //      std::size_t actual_read_size = input.readsome(unp.buffer(), try_read_size);
    qint64 actual_read_size = m_socket->read(unp.buffer(), try_read_size);
    qDebug() << actual_read_size;
    if (actual_read_size == 0) {
      break;
    } else if (actual_read_size == -1) {
      qCritical("unable to read a socket. %s", qPrintable(m_socket->errorString()));
      break;
    }

    // tell msgpack::unpacker actual consumed size.
    unp.buffer_consumed(actual_read_size);

    msgpack::unpacked result;
    // Message pack data loop
    while (unp.next(result)) {
      msgpack::object obj(result.get());
      std::string str = obj.as<std::string>();
      qDebug() << qPrintable(QString::fromUtf8(str.c_str()));
    }
  }
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
