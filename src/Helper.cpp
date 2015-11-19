#include <unordered_map>
#include <string>
#include <vector>
#include <tuple>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QKeyEvent>
#include <QMessageBox>
#include <QObject>

#include "Helper_p.h"
#include "SilkApp.h"
#include "API.h"
#include "CommandManager.h"
#include "KeymapManager.h"
#include "core/Constants.h"
#include "core/modifiers.h"
#include "core/Config.h"
#include "core/Icondition.h"
#include "core/Util.h"

using core::Constants;
using core::Config;
using core::Util;
using core::ArgumentArray;
using core::UniqueObject;

namespace {

struct QVariantArgument {
  operator QGenericArgument() const {
    if (value.isValid()) {
      return QGenericArgument(value.typeName(), value.constData());
    } else {
      return QGenericArgument();
    }
  }

  QVariant value;
};

QStringList helperArgs() {
  QStringList args;
  // stop Node in debug mode
  //  args << "--debug-brk";
  // add --harmony option first
  args << "--harmony";
  args << "--harmony_proxies";
  // first argument is main script
  args << Constants::helperDir() + "/main.js";
  // second argument is a socket path
  args << Constants::helperSocketPath();
  // third argument is locale
  args << Config::singleton().locale();
  // remaining arguments are paths to be loaded in silkedit_helper
  args << QDir::toNativeSeparators(QApplication::applicationDirPath() + "/packages");
  args << QDir::toNativeSeparators(Constants::silkHomePath() + "/packages");
  return args;
}
}

std::unordered_map<QString, std::function<void(const QString&, const msgpack::object&)>>
    HelperPrivate::s_notifyFunctions;
std::unordered_map<
    QString,
    std::function<void(msgpack::rpc::msgid_t, const QString&, const msgpack::object&)>>
    HelperPrivate::s_requestFunctions;
msgpack::rpc::msgid_t Helper::s_msgId = 0;
std::unordered_map<msgpack::rpc::msgid_t, ResponseResult*> Helper::s_eventLoopMap;

HelperPrivate::~HelperPrivate() {
  qDebug("~HelperPrivate");
  if (m_helperProcess) {
    disconnect(m_helperProcess.get(), static_cast<void (QProcess::*)(int)>(&QProcess::finished),
               this, &HelperPrivate::onFinished);
    m_helperProcess->terminate();
  }
}

void HelperPrivate::startPluginRunnerProcess() {
  m_helperProcess->start(Constants::helperPath(), helperArgs());
}

void HelperPrivate::init() {
  Q_ASSERT(!m_helperProcess);

  TextEditViewKeyHandler::singleton().registerKeyEventFilter(this);
  CommandManager::singleton().addEventFilter(std::bind(
      &HelperPrivate::cmdEventFilter, this, std::placeholders::_1, std::placeholders::_2));

  m_server = new QLocalServer(this);
  QFile socketFile(Constants::helperSocketPath());
  if (socketFile.exists()) {
    socketFile.remove();
  }

  if (!m_server->listen(Constants::helperSocketPath())) {
    qCritical("Unable to start the server: %s", qPrintable(m_server->errorString()));
    return;
  }

  connect(m_server, &QLocalServer::newConnection, this, &HelperPrivate::helperConnected);

  m_helperProcess.reset(new QProcess(this));
  connect(m_helperProcess.get(), &QProcess::readyReadStandardOutput, this,
          &HelperPrivate::readStdout);
  connect(m_helperProcess.get(), &QProcess::readyReadStandardError, this,
          &HelperPrivate::readStderr);
  connect(m_helperProcess.get(), static_cast<void (QProcess::*)(int)>(&QProcess::finished), this,
          &HelperPrivate::onFinished);
  qDebug("helper: %s", qPrintable(Constants::helperPath()));
  qDebug() << "args:" << helperArgs();
  // Disable stdout to speed up calling external command
  //  m_helperProcess->setStandardOutputFile(QProcess::nullDevice());

  startPluginRunnerProcess();
}

void HelperPrivate::sendError(const std::string& err, msgpack::rpc::msgid_t id) {
  if (q->m_isStopped)
    return;

  msgpack::sbuffer sbuf;
  msgpack::rpc::msg_response<msgpack::type::nil, std::string> response;
  response.msgid = id;
  response.error = err;
  response.result = msgpack::type::nil();

  msgpack::pack(sbuf, response);

  q->m_socket->write(sbuf.data(), sbuf.size());
}

HelperPrivate::HelperPrivate(Helper* q_ptr)
    : q(q_ptr),
      m_helperProcess(nullptr),
      m_server(nullptr),
      m_classMethodHash(QHash<QString, QHash<QString, int>>()) {}

void HelperPrivate::readStdout() {
  QProcess* p = (QProcess*)sender();
  QByteArray buf = p->readAllStandardOutput();
  qDebug() << buf;
}

void HelperPrivate::readStderr() {
  QProcess* p = (QProcess*)sender();
  QByteArray buf = p->readAllStandardError();
  qWarning() << buf;
}

void HelperPrivate::helperConnected() {
  qDebug() << "new helper process connected";

  q->m_socket = m_server->nextPendingConnection();
  Q_ASSERT(q->m_socket);
  connect(q->m_socket, &QLocalSocket::disconnected, q->m_socket, &QLocalSocket::deleteLater);

  // QueuedConnection is necessary to emit readyRead() recursively.
  // > readyRead() is not emitted recursively; if you reenter the event loop or call
  // waitForReadyRead() inside a slot connected to the readyRead() signal, the signal will not be
  // reemitted (although waitForReadyRead() may still return true).
  connect(q->m_socket, &QLocalSocket::readyRead, this, &HelperPrivate::readRequest,
          Qt::QueuedConnection);
  connect(q->m_socket,
          static_cast<void (QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error),
          this, &HelperPrivate::displayError);
}

void HelperPrivate::onFinished(int exitCode) {
  qWarning("silkedit_helper has stopped working. exit code: %d", exitCode);
  q->m_isStopped = true;
  auto reply = QMessageBox::question(
      nullptr, "Error",
      tr("silkedit_helper process has crashed. SilkEdit can continue to run but "
         "you can't use any packages. Do you want to restart the silkedit_helper process?"));
  if (reply == QMessageBox::Yes) {
    startPluginRunnerProcess();
    q->m_isStopped = false;
  }
}

void HelperPrivate::readRequest() {
  //  qDebug("readRequest");

  msgpack::unpacker unpacker;
  std::size_t readSize = q->m_socket->bytesAvailable();

  // Message receive loop
  while (true) {
    unpacker.reserve_buffer(readSize);
    // unp has at least readSize buffer on this point.

    // read message to msgpack::unpacker's internal buffer directly.
    qint64 actual_read_size = q->m_socket->read(unpacker.buffer(), readSize);
    //    qDebug() << actual_read_size;
    QByteArray array(unpacker.buffer(), actual_read_size);
    //    qDebug() << QString(array.toHex());
    if (actual_read_size == 0) {
      break;
    } else if (actual_read_size == -1) {
      qCritical("unable to read a socket. %s", qPrintable(q->m_socket->errorString()));
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
            auto found = Helper::s_eventLoopMap.find(res.msgid);
            if (found != Helper::s_eventLoopMap.end()) {
              //              qDebug("result of %d arrived", res.msgid);
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
        qCritical() << "type error. bad cast. cause: " << e.what();
        continue;
      }
    }
  }
}

void HelperPrivate::displayError(QLocalSocket::LocalSocketError socketError) {
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
      qWarning("The following error occurred: %s.", qPrintable(q->m_socket->errorString()));
      break;
  }
}

std::tuple<bool, std::string, CommandArgument> HelperPrivate::cmdEventFilter(
    const std::string& name,
    const CommandArgument& arg) {
  qDebug("cmdEventFilter");
  std::tuple<std::string, CommandArgument> event = std::make_tuple(name, arg);
  try {
    return std::move(q->sendRequest<std::tuple<std::string, CommandArgument>,
                                    std::tuple<bool, std::string, CommandArgument>>(
        "cmdEventFilter", event, msgpack::type::ARRAY));
  } catch (const std::exception& e) {
    qCritical() << e.what();
    return std::make_tuple(false, "", CommandArgument());
  }
}

bool HelperPrivate::keyEventFilter(QKeyEvent* event) {
  //  qDebug("keyEventFilter");
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
    return q->sendRequest<std::tuple<std::string, std::string, bool, bool, bool, bool, bool>, bool>(
        "keyEventFilter", params, msgpack::type::BOOLEAN);
  } catch (const std::exception& e) {
    qCritical() << e.what();
    return false;
  }
}

void ResponseResult::setResult(std::unique_ptr<object_with_zone> obj) {
  //  qDebug("setResult");
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

Helper::~Helper() {
  qDebug("~Helper");
}

void Helper::init() {
  d->init();
}

void Helper::sendFocusChangedEvent(const QString& viewType) {
  std::string type = viewType.toUtf8().constData();
  std::tuple<std::string> params = std::make_tuple(type);
  sendNotification("focusChanged", params);
}

void Helper::sendCommandEvent(const QString& command, const CommandArgument& args) {
  std::string methodName = command.toUtf8().constData();
  std::tuple<std::string, CommandArgument> params = std::make_tuple(methodName, args);
  sendNotification("commandEvent", params);
}

void Helper::callExternalCommand(const QString& cmd, const CommandArgument& args) {
  Util::stopWatch([&] {
    std::string methodName = cmd.toUtf8().constData();
    std::tuple<std::string, CommandArgument> params = std::make_tuple(methodName, args);
    try {
      sendRequest<std::tuple<std::string, CommandArgument>, bool>("runCommand", params,
                                                                  msgpack::type::BOOLEAN, -1);
    } catch (const std::exception& e) {
      qWarning("cmd: %s:, cause: %s", qPrintable(cmd), e.what());
    }
  }, cmd + " time");
}

bool Helper::askExternalCondition(const QString& name, core::Operator op, const QString& value) {
  qDebug("askExternalCondition");
  std::tuple<std::string, std::string, std::string> params = std::make_tuple(
      name.toUtf8().constData(), core::ICondition::operatorString(op).toUtf8().constData(),
      value.toUtf8().constData());

  try {
    return sendRequest<std::tuple<std::string, std::string, std::string>, bool>(
        "askCondition", params, msgpack::type::BOOLEAN);
  } catch (const std::exception& e) {
    qWarning() << e.what();
    return false;
  }
}

QString Helper::translate(const QString& key, const QString& defaultValue) {
  try {
    const std::string& result = sendRequest<std::tuple<std::string, std::string>, std::string>(
        "translate", std::make_tuple(key.toUtf8().constData(), defaultValue.toUtf8().constData()),
        msgpack::type::STR);
    return QString::fromUtf8(result.c_str());
  } catch (const std::exception& e) {
    qWarning() << e.what();
    return defaultValue;
  }
}

void Helper::loadPackage(const QString& pkgName) {
  const QString& pkgDirPath = Constants::userNodeModulesPath() + QDir::separator() + pkgName;
  const std::tuple<std::string>& params =
      std::make_tuple<std::string>(pkgDirPath.toUtf8().constData());
  sendNotification("loadPackage", params);
}

bool Helper::removePackage(const QString& pkgName) {
  const QString& pkgDirPath = Constants::userNodeModulesPath() + QDir::separator() + pkgName;
  const std::tuple<std::string>& params =
      std::make_tuple<std::string>(pkgDirPath.toUtf8().constData());

  try {
    return sendRequest<std::tuple<std::string>, bool>("removePackage", params,
                                                      msgpack::type::BOOLEAN, 0);
  } catch (const std::exception& e) {
    qWarning() << e.what();
    return false;
  }
}

GetRequestResponse* Helper::sendGetRequest(const QString& url, int timeoutInMs) {
  const std::tuple<std::string>& params = std::make_tuple<std::string>(url.toUtf8().constData());

  try {
    GetRequestResponse* response = new GetRequestResponse();
    sendRequestAsync<std::tuple<std::string>, std::string>(
        "sendGetRequest", params, msgpack::type::STR,
        [=](const std::string& body) {
          emit response->onSucceeded(QString::fromUtf8(body.c_str()));
        },
        [=](const QString& error) { emit response->onFailed(error); }, timeoutInMs);
    return response;
  } catch (const std::exception& e) {
    qWarning() << e.what();
  }
  return nullptr;
}

void Helper::reloadKeymaps() {
  sendNotification("reloadKeymaps");
}

Helper::Helper() : d(new HelperPrivate(this)), m_isStopped(false), m_socket(nullptr) {}

void HelperPrivate::cacheMethods(const QString& className, const QMetaObject* metaObj) {
  QHash<QString, int> methodNameIndexHash;
  for (int i = 0; i < metaObj->methodCount(); i++) {
    const QString& name = QString::fromLatin1(metaObj->method(i).name());
    methodNameIndexHash.insert(name, i);
  }
  m_classMethodHash.insert(className, methodNameIndexHash);
}

QVariant HelperPrivate::invokeMethod(QObject* object,
                                     const QString& methodName,
                                     QVariantList args) {
  if (args.size() > 10) {
    qWarning() << "Can't invoke method with more than 10 arguments. args:" << args.size();
    return QVariant();
  }

  if (!object) {
    QString errMsg = QString("can't convert to QObject");
    qWarning() << qPrintable(errMsg);
    return QVariant();
  }

  int methodIndex = -1;
  const QString& className = object->metaObject()->className();
  if (!m_classMethodHash.contains(className)) {
    cacheMethods(className, object->metaObject());
  }

  if (m_classMethodHash.value(className).contains(methodName)) {
    methodIndex = m_classMethodHash.value(className).value(methodName);
  }

  if (methodIndex == -1) {
    QString errMsg = QString("method: %1 not found").arg(methodName);
    qWarning() << qPrintable(errMsg);
    return QVariant();
  }

  QMetaMethod method = object->metaObject()->method(methodIndex);

  if (!method.isValid()) {
    qWarning() << "Invalid method. name:" << method.name() << "index:" << methodIndex;
    return QVariant();
  } else if (method.access() != QMetaMethod::Public) {
    qWarning() << "Can't invoke non-public method. name:" << method.name()
               << "index:" << methodIndex;
    return QVariant();
  } else if (args.size() > method.parameterCount()) {
    qWarning() << "# of arguments is more than # of parameters. name:" << method.name()
               << "args size:" << args.size() << ",parameters size:" << method.parameterCount();
  }

  QVariantArgument varArgs[10];
  for (int i = 0; i < args.size(); i++) {
    varArgs[i].value = args[i];
  }

  // Init return value
  QVariant returnValue;
  // If we pass nullptr as QVariant data, the function with QList<Window*> return type crashes.
  if (method.returnType() == qMetaTypeId<QList<Window*>>()) {
    returnValue = QVariant::fromValue(QList<Window*>());
  } else if (method.returnType() != qMetaTypeId<QVariant>() &&
             method.returnType() != qMetaTypeId<void>()) {
    returnValue = QVariant(method.returnType(), nullptr);
  }

  bool result = method.invoke(object, QGenericReturnArgument(method.typeName(), returnValue.data()),
                              varArgs[0], varArgs[1], varArgs[2], varArgs[3], varArgs[4],
                              varArgs[5], varArgs[6], varArgs[7], varArgs[8], varArgs[9]);
  if (!result) {
    qWarning("invoke %s failed", qPrintable(methodName));
  }
  return returnValue;
}

void HelperPrivate::callNotifyFunc(const QString& method, const msgpack::v1::object& obj) {
  ArgumentArray params;
  obj.convert(&params);
  int id = params.id();

  QObject* view;
  // API id is -1
  if (id == -1) {
    view = &API::singleton();
  } else {
    view = UniqueObject::find(id);
  }

  if (view) {
    invokeMethod(view, method, params.args());
  } else {
    qWarning("id: %d not found", id);
  }
}

void HelperPrivate::callRequestFunc(const QString& method,
                                    msgpack::rpc::msgid_t msgId,
                                    const msgpack::v1::object& obj) {
  ArgumentArray params;
  obj.convert(&params);
  int id = params.id();

  QObject* view;
  // API id is -1
  if (id == -1) {
    view = &API::singleton();
  } else {
    view = UniqueObject::find(id);
  }

  if (view) {
    const QVariant& returnValue = invokeMethod(view, method, params.args());
    Helper::singleton().sendResponse(returnValue, msgpack::type::nil(), msgId);
  } else {
    QString errMsg = QString("id: %1 not found").arg(id);
    qWarning() << qPrintable(errMsg);
    Helper::singleton().sendResponse(msgpack::type::nil(),
                                     ((std::string)errMsg.toUtf8().constData()), msgId);
  }
}

std::tuple<bool, std::string, CommandArgument> Helper::cmdEventFilter(const std::string& name,
                                                                      const CommandArgument& arg) {
  qDebug("cmdEventFilter");
  std::tuple<std::string, CommandArgument> event = std::make_tuple(name, arg);
  try {
    return std::move(sendRequest<std::tuple<std::string, CommandArgument>,
                                 std::tuple<bool, std::string, CommandArgument>>(
        "cmdEventFilter", event, msgpack::type::ARRAY));
  } catch (const std::exception& e) {
    qWarning() << e.what();
    return std::make_tuple(false, "", CommandArgument());
  }
}
