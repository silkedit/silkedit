#include <msgpack/rpc/protocol.h>
#include <msgpack.hpp>
#include <QProcess>
#include <QDebug>
#include <QFile>

#include "PluginService.h"
#include "SilkApp.h"
#include "Constants.h"

PluginService::~PluginService() {
  qDebug("~PluginService");
  if (m_pluginProcess) {
    m_pluginProcess->terminate();
  }
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

  //  m_pluginProcess = new QProcess(this);
  //  connect(m_pluginProcess,
  //          SIGNAL(error(QProcess::ProcessError)),
  //          this,
  //          SLOT(error(QProcess::ProcessError)));
  //  connect(m_pluginProcess, &QProcess::readyReadStandardOutput, this,
  // &PluginService::readStdout);
  qDebug("plugin runner: %s", qPrintable(Constants::pluginRunnerPath()));
  qDebug() << "args:" << Constants::pluginRunnerArgs();
  //  m_pluginProcess->start(Constants::pluginRunnerPath(), Constants::pluginRunnerArgs());
}

PluginService::PluginService() : m_pluginProcess(nullptr), m_socket(nullptr), m_server(nullptr) {}

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

void PluginService::error(QProcess::ProcessError error) { qDebug() << "Error: " << error; }

void PluginService::readRequest() {
  qDebug("readRequest");

  msgpack::unpacker unp;
  std::size_t readSize = m_socket->bytesAvailable();

  // Message receive loop
  while (true) {
    unp.reserve_buffer(readSize);
    // unp has at least readSize buffer on this point.

    // read message to msgpack::unpacker's internal buffer directly.
    qint64 actual_read_size = m_socket->read(unp.buffer(), readSize);
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
      msgpack::rpc::msg_rpc rpc;
      try {
        obj.convert(&rpc);
      }
      catch (msgpack::v1::type_error e) {
        qCritical() << "type error. bad cast.";
        continue;
      }

      switch (rpc.type) {
        case ::msgpack::rpc::REQUEST: {
          msgpack::rpc::msg_request<msgpack::object, msgpack::object> req;
          obj.convert(&req);
          std::string methodName = req.method.as<std::string>();
          if (methodName == "add") {
            qDebug() << qPrintable(QString::fromUtf8(methodName.c_str()));
            msgpack::type::tuple<int, int> params;
            req.param.convert(&params);
            if (req.param.type == msgpack::type::ARRAY) {
              qDebug("array type");
              qDebug() << req.param.via.array.size;
            }
            int a = std::get<0>(params);
            int b = std::get<1>(params);
            qDebug() << a;
            qDebug() << b;
            msgpack::type::nil err = msgpack::type::nil();
            //            msgpack::type::nil res = msgpack::type::nil();
            int res = a + b;

            call(res, err, req.msgid);
          } else {
            qWarning("%s is not supported", qPrintable(QString::fromUtf8(methodName.c_str())));
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
          msgpack::rpc::msg_notify<msgpack::object, msgpack::object> req;
          obj.convert(&req);
        } break;

        default:
          qCritical("invalid rpc type");
      }
    }
  }
}

template <typename Result, typename Error>
void PluginService::call(Result& res, Error& err, msgpack::rpc::msgid_t id) {
  msgpack::sbuffer sbuf;
  msgpack::type::tuple<msgpack::rpc::message_type_t, msgpack::rpc::msgid_t, Error, Result> response;
  std::get<0>(response) = msgpack::rpc::RESPONSE;
  std::get<1>(response) = id;
  std::get<2>(response) = err;
  std::get<3>(response) = res;
  msgpack::pack(sbuf, response);

  m_socket->write(sbuf.data(), sbuf.size());
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
