#include <unordered_map>
#include <string>
#include <vector>
#include <tuple>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QKeyEvent>
#include <QMessageBox>

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
#include "CommandManager.h"
#include "modifiers.h"

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
    disconnect(m_pluginProcess.get(), static_cast<void (QProcess::*)(int)>(&QProcess::finished),
               this, &PluginManager::onFinished);
    m_pluginProcess->terminate();
  }
}

void PluginManager::startPluginRunnerProcess() {
  m_pluginProcess->start(Constants::pluginRunnerPath(), Constants::pluginRunnerArgs());
}

void PluginManager::init() {
  Q_ASSERT(!m_pluginProcess);

  TextEditViewKeyHandler::singleton().registerKeyEventFilter(this);
  CommandManager::addEventFilter(std::bind(&PluginManager::cmdEventFilter, this,
                                           std::placeholders::_1, std::placeholders::_2));

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

  m_pluginProcess.reset(new QProcess(this));
  connect(m_pluginProcess.get(), &QProcess::readyReadStandardOutput, this,
          &PluginManager::readStdout);
  connect(m_pluginProcess.get(), &QProcess::readyReadStandardError, this,
          &PluginManager::readStderr);
  connect(m_pluginProcess.get(), static_cast<void (QProcess::*)(int)>(&QProcess::finished), this,
          &PluginManager::onFinished);
  qDebug("plugin runner: %s", qPrintable(Constants::pluginRunnerPath()));
  qDebug() << "args:" << Constants::pluginRunnerArgs();
  // Disable stdout. With stdout, main.js (Node 0.12) doesn't work correctly on Windows 7 64 bit.
  m_pluginProcess->setStandardOutputFile(QProcess::nullDevice());
  startPluginRunnerProcess();
}

void PluginManager::sendError(const std::string& err, msgpack::rpc::msgid_t id) {
  if (m_isStopped)
    return;

  msgpack::sbuffer sbuf;
  msgpack::rpc::msg_response<msgpack::type::nil, std::string> response;
  response.msgid = id;
  response.error = err;
  response.result = msgpack::type::nil();

  msgpack::pack(sbuf, response);

  m_socket->write(sbuf.data(), sbuf.size());
}

void PluginManager::sendFocusChangedEvent(const QString& viewType) {
  std::string type = viewType.toUtf8().constData();
  std::tuple<std::string> params = std::make_tuple(type);
  sendNotification("focusChanged", params);
}

void PluginManager::sendCommandEvent(const QString& command, const CommandArgument& args) {
  std::string methodName = command.toUtf8().constData();
  std::tuple<std::string, CommandArgument> params = std::make_tuple(methodName, args);
  sendNotification("commandEvent", params);
}

void PluginManager::callExternalCommand(const QString& cmd, const CommandArgument& args) {
  std::string methodName = cmd.toUtf8().constData();
  std::tuple<std::string, CommandArgument> params = std::make_tuple(methodName, args);
  sendNotification("runCommand", params);
}

bool PluginManager::askExternalContext(const QString& name, Operator op, const QString& value) {
  qDebug("askExternalContext");
  std::tuple<std::string, std::string, std::string> params =
      std::make_tuple(name.toUtf8().constData(), IContext::operatorString(op).toUtf8().constData(),
                      value.toUtf8().constData());

  try {
    return sendRequest<std::tuple<std::string, std::string, std::string>, bool>(
        "askContext", params, msgpack::type::BOOLEAN);
  } catch (const std::exception& e) {
    qWarning() << e.what();
    return false;
  }
}

PluginManager::PluginManager()
    : m_pluginProcess(nullptr), m_socket(nullptr), m_server(nullptr), m_isStopped(false) {
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
  connect(m_socket, &QLocalSocket::readyRead, this, &PluginManager::readRequest,
          Qt::QueuedConnection);
  connect(m_socket,
          static_cast<void (QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error),
          this, &PluginManager::displayError);
}

void PluginManager::onFinished(int exitCode) {
  qWarning("plugin runner has stopped working. exit code: %d", exitCode);
  m_isStopped = true;
  auto reply =
      QMessageBox::question(nullptr, "Error",
                            "Plugin runner has stopped working. SilkEdit can continue to run but "
                            "you can't use any plugins. Do you want to restart the plugin runner?");
  if (reply == QMessageBox::Yes) {
    startPluginRunnerProcess();
    m_isStopped = false;
  }
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
  std::string type = event->type() & QEvent::KeyPress ? "keypress" : "keyup";
  bool isModifierKey = false;
  std::string key;
  switch (event->key()) {
    case Silk::Key_Alt:
      isModifierKey = true;
      key = "Alt";
      break;
    case Silk::Key_Control:
      isModifierKey = true;
      key = "Control";
      break;
    case Silk::Key_Meta:
      isModifierKey = true;
      key = "Meta";
      break;
    case Silk::Key_Shift:
      isModifierKey = true;
      key = "Shift";
      break;
    default:
      key = QKeySequence(event->key()).toString().toLower().toUtf8().constData();
      break;
  }

  // don't send keypress/up event if only a modifier key is pressed
  if (isModifierKey) {
    return false;
  }

  bool altKey = event->modifiers() & Silk::AltModifier;
  bool ctrlKey = event->modifiers() & Silk::ControlModifier;
  bool metaKey = event->modifiers() & Silk::MetaModifier;
  bool shiftKey = event->modifiers() & Silk::ShiftModifier;

  std::tuple<std::string, std::string, bool, bool, bool, bool, bool> params =
      std::make_tuple(type, key, event->isAutoRepeat(), altKey, ctrlKey, metaKey, shiftKey);
  try {
    return sendRequest<std::tuple<std::string, std::string, bool, bool, bool, bool, bool>, bool>(
        "keyEventFilter", params, msgpack::type::BOOLEAN);
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
