#include <unordered_map>
#include <string>
#include <vector>
#include <tuple>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QKeyEvent>

#include "PluginManager.h"
#include "SilkApp.h"
#include "Constants.h"
#include "API.h"
#include "TextEditView.h"
#include "TabView.h"
#include "Window.h"
#include "StatusBar.h"
#include "TabViewGroup.h"
#include "KeymapManager.h"

#define REGISTER_FUNC(type)                                                 \
  s_requestFunctions.insert(std::make_pair(#type, &type::callRequestFunc)); \
  s_notifyFunctions.insert(std::make_pair(#type, &type::callNotifyFunc));

std::unordered_map<QString, std::function<void(const QString&, const msgpack::object&)>>
    PluginManager::s_notifyFunctions;
std::unordered_map<
    QString,
    std::function<void(msgpack::rpc::msgid_t, const QString&, const msgpack::object&)>>
    PluginManager::s_requestFunctions;
msgpack::rpc::msgid_t PluginManager::s_msgId = 0;
std::unordered_map<msgpack::rpc::msgid_t, ResponseResult*> PluginManager::s_eventLoopMap;

PluginManager::~PluginManager() {
  qDebug("~PluginManager");
  if (m_pluginProcess) {
    m_pluginProcess->terminate();
  }
}

void PluginManager::init() {
  Q_ASSERT(!m_pluginProcess);

  KeyHandler::singleton().registerKeyEventFilter(this);
  CommandManager::addEventFilter(std::bind(
      &PluginManager::cmdEventFilter, this, std::placeholders::_1, std::placeholders::_2));

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
  m_pluginProcess->setArguments(QStringList("--harmony"));
  m_pluginProcess->start(Constants::pluginRunnerPath(), Constants::pluginRunnerArgs());
}

void PluginManager::callExternalCommand(const QString& cmd, CommandArgument args) {
  msgpack::sbuffer sbuf;
  msgpack::rpc::msg_notify<std::string,
                           std::tuple<std::string, std::unordered_map<std::string, std::string>>>
      notify;
  notify.method = "runCommand";
  std::string methodName = cmd.toUtf8().constData();
  std::tuple<std::string, CommandArgument> params = std::make_tuple(methodName, args);
  notify.param = params;
  msgpack::pack(sbuf, notify);

  m_socket->write(sbuf.data(), sbuf.size());
}

bool PluginManager::askExternalContext(const QString& name, Operator op, const QString& value) {
  qDebug("askExternalContext");
  std::tuple<std::string, std::string, std::string> params =
      std::make_tuple(name.toUtf8().constData(),
                      IContext::operatorString(op).toUtf8().constData(),
                      value.toUtf8().constData());

  try {
    return sendRequest<std::tuple<std::string, std::string, std::string>, bool>(
        "askContext", params, msgpack::type::BOOLEAN);
  } catch (const std::exception& e) {
    qWarning() << e.what();
    return false;
  }
}

PluginManager::PluginManager() : m_pluginProcess(nullptr), m_socket(nullptr), m_server(nullptr) {
  REGISTER_FUNC(TextEditView)
  REGISTER_FUNC(TabView)
  REGISTER_FUNC(TabViewGroup)
  REGISTER_FUNC(Window)
  REGISTER_FUNC(StatusBar)
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
  connect(m_socket, &QLocalSocket::disconnected, m_socket, &QLocalSocket::deleteLater);

  // QueuedConnection is necessary to emit readyRead() recursively.
  // > readyRead() is not emitted recursively; if you reenter the event loop or call
  // waitForReadyRead() inside a slot connected to the readyRead() signal, the signal will not be
  // reemitted (although waitForReadyRead() may still return true).
  connect(
      m_socket, &QLocalSocket::readyRead, this, &PluginManager::readRequest, Qt::QueuedConnection);
  connect(m_socket,
          static_cast<void (QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error),
          this,
          &PluginManager::displayError);
}

void PluginManager::error(QProcess::ProcessError error) {
  qDebug() << "Error: " << error;
}

void PluginManager::readRequest() {
  qDebug("readRequest");

  msgpack::unpacker unpacker;
  std::size_t readSize = m_socket->bytesAvailable();

  // Message receive loop
  while (true) {
    unpacker.reserve_buffer(readSize);
    // unp has at least readSize buffer on this point.

    // read message to msgpack::unpacker's internal buffer directly.
    qint64 actual_read_size = m_socket->read(unpacker.buffer(), readSize);
    //    qDebug() << actual_read_size;
    QByteArray array(unpacker.buffer(), actual_read_size);
    qDebug() << QString(array.toHex());
    if (actual_read_size == 0) {
      break;
    } else if (actual_read_size == -1) {
      qCritical("unable to read a socket. %s", qPrintable(m_socket->errorString()));
      break;
    }

    // tell msgpack::unpacker actual consumed size.
    unpacker.buffer_consumed(actual_read_size);

    msgpack::unpacked result;
    // Message pack data loop
    while (unpacker.next(result)) {
      msgpack::object obj(result.get());
      msgpack::rpc::msg_rpc rpc;
      try {
        obj.convert(&rpc);

        switch (rpc.type) {
          case msgpack::rpc::REQUEST: {
            msgpack::rpc::msg_request<msgpack::object, msgpack::object> req;
            obj.convert(&req);
            QString methodName = QString::fromUtf8(req.method.as<std::string>().c_str());
            callRequestFunc(methodName, req.msgid, req.param);
          } break;

          case msgpack::rpc::RESPONSE: {
            msgpack::rpc::msg_response<msgpack::object, msgpack::object> res;
            obj.convert(&res);
            auto found = s_eventLoopMap.find(res.msgid);
            if (found != s_eventLoopMap.end()) {
              qDebug("result of %d arrived", res.msgid);
              if (res.error.type == msgpack::type::NIL) {
                found->second->setResult(std::move(std::unique_ptr<object_with_zone>(
                    new object_with_zone(res.result, std::move(result.zone())))));
              } else {
                found->second->setError(std::move(std::unique_ptr<object_with_zone>(
                    new object_with_zone(res.error, std::move(result.zone())))));
              }
            } else {
              qWarning("no matched response result for %d", res.msgid);
            }
          } break;

          case msgpack::rpc::NOTIFY: {
            msgpack::rpc::msg_notify<msgpack::object, msgpack::object> notify;
            obj.convert(&notify);
            QString methodName = QString::fromUtf8(notify.method.as<std::string>().c_str());
            callNotifyFunc(methodName, notify.param);
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

std::tuple<bool, std::string, CommandArgument> PluginManager::cmdEventFilter(
    const std::string& name,
    const CommandArgument& arg) {
  qDebug("cmdEventFilter");
  std::tuple<std::string, CommandArgument> event = std::make_tuple(name, arg);
  try {
    return std::move(sendRequest<std::tuple<std::string, CommandArgument>,
                                 std::tuple<bool, std::string, CommandArgument>>(
        "cmdEventFilter", event, msgpack::type::ARRAY));
  } catch (const std::exception& e) {
    qCritical() << e.what();
    return std::make_tuple(false, "", CommandArgument());
  }
}

void PluginManager::callRequestFunc(const QString& methodName,
                                    msgpack::rpc::msgid_t msgId,
                                    const msgpack::object& obj) {
  qDebug() << "method:" << qPrintable(methodName);
  int dotIndex = methodName.indexOf(".");
  if (dotIndex >= 0) {
    QString type = methodName.mid(0, dotIndex);
    QString method = methodName.mid(dotIndex + 1);
    if (type.isEmpty() || method.isEmpty()) {
      return;
    }

    qDebug("type: %s, method: %s", qPrintable(type), qPrintable(method));
    if (s_requestFunctions.count(type) != 0) {
      s_requestFunctions.at(type)(msgId, method, obj);
    } else {
      qWarning("type: %s not supported", qPrintable(type));
    }
  } else {
    API::call(methodName, msgId, obj);
  }
}

void PluginManager::callNotifyFunc(const QString& methodName, const msgpack::object& obj) {
  qDebug() << "method:" << qPrintable(methodName);
  if (obj.type != msgpack::type::ARRAY) {
    qWarning("params must be an array");
    return;
  }

  int dotIndex = methodName.indexOf(".");
  if (dotIndex >= 0) {
    QString type = methodName.mid(0, dotIndex);
    QString method = methodName.mid(dotIndex + 1);
    if (type.isEmpty() || method.isEmpty()) {
      return;
    }

    qDebug("type: %s, method: %s", qPrintable(type), qPrintable(method));
    if (s_notifyFunctions.count(type) != 0) {
      s_notifyFunctions.at(type)(method, obj);
    } else {
      qWarning("type: %s not supported", qPrintable(type));
    }
  } else {
    API::call(methodName, obj);
  }
}

bool PluginManager::keyEventFilter(QKeyEvent* event) {
  qDebug("keyEventFilter");
  std::unordered_map<std::string, std::string> map;
  map.insert(std::make_pair("key", event->text().toUtf8().constData()));
  std::tuple<std::string, std::unordered_map<std::string, std::string>> params =
      std::make_tuple("keypress", map);
  try {
    return sendRequest<std::tuple<std::string, std::unordered_map<std::string, std::string>>, bool>(
        "eventFilter", params, msgpack::type::BOOLEAN);
  } catch (const std::exception& e) {
    qCritical() << e.what();
    return false;
  }
}

void ResponseResult::setResult(std::unique_ptr<object_with_zone> obj) {
  qDebug("setResult");
  m_isReady = true;
  m_isSuccess = true;
  m_result = std::move(obj);
  emit ready();
}

void ResponseResult::setError(std::unique_ptr<object_with_zone> obj) {
  qDebug("setError");
  m_isReady = true;
  m_isSuccess = false;
  m_result = std::move(obj);
  emit ready();
}
