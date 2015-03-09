#include <vector>
#include <tuple>
#include <msgpack/rpc/protocol.h>
#include <msgpack.hpp>
#include <QProcess>
#include <QDebug>
#include <QFile>

#include "PluginManager.h"
#include "SilkApp.h"
#include "Constants.h"
#include "API.h"
#include "TextEditView.h"

PluginManager::~PluginManager() {
  qDebug("~PluginManager");
  if (m_pluginProcess) {
    m_pluginProcess->terminate();
  }
}

void PluginManager::init() {
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

  connect(m_server, &QLocalServer::newConnection, this, &PluginManager::pluginRunnerConnected);

  m_pluginProcess = new QProcess(this);
  connect(m_pluginProcess,
          SIGNAL(error(QProcess::ProcessError)),
          this,
          SLOT(error(QProcess::ProcessError)));
  connect(m_pluginProcess, &QProcess::readyReadStandardOutput, this, &PluginManager::readStdout);
  connect(m_pluginProcess, &QProcess::readyReadStandardError, this, &PluginManager::readStderr);
  qDebug("plugin runner: %s", qPrintable(Constants::pluginRunnerPath()));
  qDebug() << "args:" << Constants::pluginRunnerArgs();
  // Disable stdout. With stdout, main.js (Node 0.12) doesn't work correctly on Windows 7 64 bit.
  m_pluginProcess->setStandardOutputFile(QProcess::nullDevice());
  m_pluginProcess->start(Constants::pluginRunnerPath(), Constants::pluginRunnerArgs());
}

PluginManager::PluginManager() : m_pluginProcess(nullptr), m_socket(nullptr), m_server(nullptr) {
}

void PluginManager::callExternalCommand(const QString& cmd) {
  msgpack::sbuffer sbuf;
  msgpack::rpc::msg_notify<std::string, std::vector<std::string>> notify;
  notify.method = "runCommand";
  std::vector<std::string> params = {cmd.toUtf8().constData()};
  notify.param = params;
  msgpack::pack(sbuf, notify);

  m_socket->write(sbuf.data(), sbuf.size());
}

void PluginManager::readStdout() {
  QProcess* p = (QProcess*)sender();
  QByteArray buf = p->readAllStandardOutput();
  qDebug() << buf;
}

void PluginManager::readStderr() {
  QProcess* p = (QProcess*)sender();
  QByteArray buf = p->readAllStandardError();
  qWarning() << buf;
}

void PluginManager::pluginRunnerConnected() {
  qDebug() << "new Plugin runner connected";

  m_socket = m_server->nextPendingConnection();
  connect(m_socket, SIGNAL(disconnected()), m_socket, SLOT(deleteLater()));
  connect(m_socket, &QLocalSocket::readyRead, this, &PluginManager::readRequest);
  connect(m_socket,
          SIGNAL(error(QLocalSocket::LocalSocketError)),
          this,
          SLOT(displayError(QLocalSocket::LocalSocketError)));
}

void PluginManager::error(QProcess::ProcessError error) {
  qDebug() << "Error: " << error;
}

void PluginManager::readRequest() {
  qDebug("readRequest");

  msgpack::unpacker unp;
  std::size_t readSize = m_socket->bytesAvailable();

  // Message receive loop
  while (true) {
    unp.reserve_buffer(readSize);
    // unp has at least readSize buffer on this point.

    // read message to msgpack::unpacker's internal buffer directly.
    qint64 actual_read_size = m_socket->read(unp.buffer(), readSize);
    //    qDebug() << actual_read_size;
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
      msgpack::rpc::msg_rpc rpc;
      try {
        obj.convert(&rpc);

        switch (rpc.type) {
          case ::msgpack::rpc::REQUEST: {
            msgpack::rpc::msg_request<msgpack::object, msgpack::object> req;
            obj.convert(&req);
            std::string methodName = req.method.as<std::string>();
            qDebug() << "method:" << qPrintable(QString::fromUtf8(methodName.c_str()));
            int dotIndex = methodName.find_first_of(".");
            if (dotIndex >= 0) {
              std::string type = methodName.substr(0, dotIndex);
              std::string method = methodName.substr(dotIndex + 1);
              if (type.empty() || method.empty()) {
                return;
              }

              qDebug("type: %s, method: %s", type.c_str(), method.c_str());
              if (type == "TextEditView") {
                TextEditView::call(req.msgid, method, req.param);
              }
            } else {
              API::call(methodName, req.msgid, req.param);
            }
          } break;

          case ::msgpack::rpc::RESPONSE: {
            //          ::msgpack::rpc::msg_response<object, object> res;
            //          msg.convert(&res);
            //          auto found = m_request_map.find(res.msgid);
            //          if (found != m_request_map.end()) {
            //            if (res.error.type == msgpack::type::NIL) {
            //              found->second->set_result(res.result);
            //            } else if (res.error.type == msgpack::type::BOOLEAN) {
            //              bool isError;
            //              res.error.convert(&isError);
            //              if (isError) {
            //                found->second->set_error(res.result);
            //              } else {
            //                found->second->set_result(res.result);
            //              }
            //            }
            //          } else {
            //            throw client_error("no request for response");
            //          }
          } break;

          case msgpack::rpc::NOTIFY: {
            msgpack::rpc::msg_notify<msgpack::object, msgpack::object> notify;
            obj.convert(&notify);
            std::string methodName = notify.method.as<std::string>();
            qDebug() << "method:" << qPrintable(QString::fromUtf8(methodName.c_str()));
            if (obj.type != msgpack::type::ARRAY) {
              qWarning("params must be an array");
              return;
            }

            API::call(methodName, notify.param);
          } break;
          default:
            qCritical("invalid rpc type");
        }
      } catch (msgpack::v1::type_error e) {
        qCritical() << "type error. bad cast.";
        continue;
      }
    }
  }
}

void PluginManager::displayError(QLocalSocket::LocalSocketError socketError) {
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
