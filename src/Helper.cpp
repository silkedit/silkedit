#include <memory>
#include <node.h>
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
#include <QApplication>
#include <QMetaObject>
#include <QMetaMethod>
#include <QObject>
#include <QEventLoop>
#include <QMutexLocker>
#include <QVariantMap>

#include "Helper_p.h"
#include "API.h"
#include "CommandManager.h"
#include "KeymapManager.h"
#include "Window.h"
#include "core/Constants.h"
#include "core/modifiers.h"
#include "core/Config.h"
#include "core/Icondition.h"
#include "core/Util.h"
#include "core/QVariantArgument.h"

using core::Constants;
using core::Config;
using core::Util;
using core::QVariantArgument;

using v8::String;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Value;
using v8::Object;
using v8::Function;
using v8::Null;
using v8::Persistent;
using v8::HandleScope;
using v8::TryCatch;
using v8::Boolean;
using v8::EscapableHandleScope;
using v8::Array;

namespace {

QStringList helperArgs() {
  QStringList args;
  args << QCoreApplication::applicationFilePath();
  // stop Node in debug mode
  //  args << "--debug-brk";
  // expose global.gc()
  //  args << "--expose-gc";
  args << "--harmony";
  args << "--harmony_proxies";
  // first argument is main script
  args << Constants::singleton().jsLibDir() + "/main.js";
  // third argument is locale
  args << Config::singleton().locale();
  // remaining arguments are paths to be loaded in silkedit_helper
  args << QDir::toNativeSeparators(QApplication::applicationDirPath() + "/packages");
  args << QDir::toNativeSeparators(Constants::singleton().silkHomePath() + "/packages");
  return args;
}

CommandArgument toCommandArgument(QVariantMap map) {
  CommandArgument arg;
  auto it = map.constBegin();
  while (it != map.constEnd()) {
    arg.insert(
        std::make_pair(it.key().toUtf8().constData(), it.value().toString().toUtf8().constData()));
  }
  return arg;
}
}

HelperPrivate::~HelperPrivate() {
  qDebug("~HelperPrivate");
}

void HelperPrivate::startHelperThread() {
  m_helperThread->start();
}

void HelperPrivate::init() {
  Q_ASSERT(!m_helperThread);

  TextEditViewKeyHandler::singleton().registerKeyEventFilter(this);
  CommandManager::singleton().addEventFilter(std::bind(
      &HelperPrivate::cmdEventFilter, this, std::placeholders::_1, std::placeholders::_2));

  m_helperThread = new HelperThread(this);
  connect(m_helperThread, &QThread::finished, this, &HelperPrivate::onFinished);

  startHelperThread();
}

HelperPrivate::HelperPrivate(Helper* q_ptr)
    : q(q_ptr), m_helperThread(nullptr) {}

void HelperPrivate::onFinished() {
  qCritical("Helper thread has stopped working.");
}

CommandEventFilterResult HelperPrivate::cmdEventFilter(const std::string& name,
                                                       const CommandArgument& arg) {
  const QVariantList& args = QVariantList{QVariant::fromValue(name), QVariant::fromValue(arg)};
  const QVariant& result = q->callRequestJSFunc("cmdEventFilter", args);
  if (result.canConvert<QVariantList>()) {
    QVariantList varResults = result.value<QVariantList>();
    if (varResults.size() == 3 && varResults[0].canConvert<bool>() &&
        varResults[1].canConvert<QString>() && varResults[2].canConvert<QVariantMap>()) {
      return std::make_tuple(varResults[0].toBool(), varResults[1].toString().toUtf8().constData(),
                             toCommandArgument(varResults[2].value<QVariantMap>()));
    }
  }
  return std::make_tuple(false, "", CommandArgument());
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

  std::unique_ptr<BoolResponse> response = std::make_unique<BoolResponse>();
  const QVariantList& args = QVariantList{QVariant::fromValue(type),
                                          QVariant::fromValue(key),
                                          QVariant::fromValue(event->isAutoRepeat()),
                                          QVariant::fromValue(altKey),
                                          QVariant::fromValue(ctrlKey),
                                          QVariant::fromValue(metaKey),
                                          QVariant::fromValue(shiftKey),
                                          QVariant::fromValue(response.get())};
  QEventLoop loop;
  connect(response.get(), &BoolResponse::finished, &loop, &QEventLoop::quit);
  q->callNotifyJSFunc("keyEventFilter", args);
  loop.exec(QEventLoop::ExcludeUserInputEvents);
  return response->result();
}

Helper::~Helper() {
  qDebug("~Helper");
  uv_close((uv_handle_t*)&async, NULL);
}

void Helper::init() {
  d->init();
}

void Helper::sendFocusChangedEvent(const QString& viewType) {
  const QVariantList& args = QVariantList{QVariant::fromValue(viewType)};
  callNotifyJSFunc("focusChanged", args);
}

void Helper::sendCommandEvent(const QString& command, const CommandArgument& cmdArgs) {
  const QVariantList& args =
      QVariantList{QVariant::fromValue(command), QVariant::fromValue(cmdArgs)};
  callNotifyJSFunc("commandEvent", args);
}

void Helper::runCommand(const QString& cmd, const CommandArgument& cmdArgs) {
  const QVariantList& args = QVariantList{QVariant::fromValue(cmd), QVariant::fromValue(cmdArgs)};
  callRequestJSFunc("runCommand", args);
}

bool Helper::askCondition(const QString& name, core::Operator op, const QString& value) {
  const QVariantList& args = QVariantList{QVariant::fromValue(name),
                                          QVariant::fromValue(core::ICondition::operatorString(op)),
                                          QVariant::fromValue(value)};
  const QVariant& result = callRequestJSFunc("askCondition", args);
  return result.canConvert<bool>() ? result.toBool() : false;
}

QString Helper::translate(const QString& key, const QString& defaultValue) {
  const QVariantList& args =
      QVariantList{QVariant::fromValue(key), QVariant::fromValue(defaultValue)};
  const QVariant& result = callRequestJSFunc("translate", args);
  return result.canConvert<QString>() ? result.toString() : defaultValue;
}

void Helper::emitSignalInternal(const QVariantList& args) {
  if (!isAsyncReady()) {
    qWarning() << "uv_async is not active";
    return;
  }

  QObject* obj = QObject::sender();
  if (!obj) {
    qWarning() << "sender is null";
    return;
  }

  if (obj->thread() != QThread::currentThread()) {
    qWarning() << "invalid thread affinity";
    return;
  }

  const QMetaMethod& method = obj->metaObject()->method(QObject::senderSignalIndex());
  if (!method.isValid()) {
    qWarning() << "signal method is invalid";
    return;
  }

  //  qDebug() << "emitSignalInternal." << method.name();
  auto emitSignal = std::make_shared<NodeEmitSignal>(obj, method.name(), args);
  nodeRemoteCalls.enqueue(QVariant::fromValue(emitSignal));
  asyncLock.lock();
  async.data = (void*)&nodeRemoteCalls;
  asyncLock.unlock();

  QEventLoop eventLoop;
  connect(emitSignal.get(), &NodeEmitSignal::finished, &eventLoop, &QEventLoop::quit);
  uv_async_send(&async);
  // don't block current thread because Node.js needs to call silkedit API in this call
  eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
}

bool Helper::isAsyncReady() {
  QMutexLocker locker(&asyncLock);
  return async.type == UV_ASYNC;
}

void Helper::emitSignal() {
  emitSignalInternal(QVariantList());
}

void Helper::callNotifyJSFunc(const QString& funcName, const QVariantList& args) {
  if (!isAsyncReady()) {
    qWarning() << "uv_async is not active";
    return;
  }

  QVariant methodCall = QVariant::fromValue(std::make_shared<NodeMethodCall>(funcName, args));
  // We need queuing NodeMethodCall request because uv_async_send may coalesce multiple
  // uv_async_send calls into single one.
  // http://docs.libuv.org/en/v1.x/async.html
  nodeRemoteCalls.enqueue(methodCall);
  asyncLock.lock();
  async.data = (void*)&nodeRemoteCalls;
  asyncLock.unlock();

  uv_async_send(&async);
}

QVariant Helper::callRequestJSFunc(const QString& funcName, const QVariantList& args) {
  if (!isAsyncReady()) {
    qWarning() << "uv_async is not active";
    return QVariant();
  }

  QVariant methodCall = QVariant::fromValue(std::make_shared<NodeMethodCall>(funcName, args));
  // We need queuing NodeMethodCall request because uv_async_send may coalesce multiple
  // uv_async_send calls into single one.
  // http://docs.libuv.org/en/v1.x/async.html
  nodeRemoteCalls.enqueue(methodCall);
  asyncLock.lock();
  async.data = (void*)&nodeRemoteCalls;
  asyncLock.unlock();

  QEventLoop eventLoop;
  connect(methodCall.value<std::shared_ptr<NodeMethodCall>>().get(), &NodeMethodCall::finished,
          &eventLoop, &QEventLoop::quit);
  uv_async_send(&async);
  // don't block current thread because Node.js needs to call silkedit API in this call
  eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
  return methodCall.value<std::shared_ptr<NodeMethodCall>>()->returnValue();
}

void Helper::loadPackage(const QString& pkgName) {
  const QString& pkgDirPath =
      Constants::singleton().userPackagesNodeModulesPath() + QDir::separator() + pkgName;
  const QVariantList& args = QVariantList{QVariant::fromValue(pkgDirPath)};
  callNotifyJSFunc("loadPackage", args);
}

bool Helper::removePackage(const QString& pkgName) {
  const QString& pkgDirPath =
      Constants::singleton().userPackagesNodeModulesPath() + QDir::separator() + pkgName;
  std::unique_ptr<BoolResponse> response = std::make_unique<BoolResponse>();
  const QVariantList& args =
      QVariantList{QVariant::fromValue(pkgDirPath), QVariant::fromValue(response.get())};
  QEventLoop loop;
  connect(response.get(), &BoolResponse::finished, &loop, &QEventLoop::quit);
  callNotifyJSFunc("removePackage", args);
  loop.exec(QEventLoop::ExcludeUserInputEvents);
  return response->result();
}

GetRequestResponse* Helper::sendGetRequest(const QString& url, int timeoutInMs) {
  GetRequestResponse* response = new GetRequestResponse();
  const QVariantList& args = QVariantList{
      QVariant::fromValue(url), QVariant::fromValue(timeoutInMs), QVariant::fromValue(response)};
  callNotifyJSFunc("sendGetRequest", args);
  return response;
}

void Helper::reloadKeymaps() {
  callNotifyJSFunc("reloadKeymaps");
}

void Helper::invokeMethodFromJS(std::shared_ptr<InvokeMethodInfo> info,
                                std::shared_ptr<UvEventLoop> loop) {
  qDebug() << "invokeMethod in main thread." << info->method().name();
  QObject* object = info->object();
  const QMetaMethod& method = info->method();
  QVariantList args = info->args();

  assert(args.size() <= Q_METAMETHOD_INVOKE_MAX_ARGS);
  assert(args.size() <= method.parameterCount());
  assert(object);
  assert(method.isValid());
  assert(method.access() == QMetaMethod::Public);

  QVariantArgument varArgs[Q_METAMETHOD_INVOKE_MAX_ARGS];
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

  QGenericReturnArgument returnArg;
  if (returnValue.isValid()) {
    returnArg = QGenericReturnArgument(method.typeName(), returnValue.data());
  }

  if (object->thread() != QCoreApplication::instance()->thread()) {
    qWarning() << "object thread affinity must be main thread";
  } else {
    bool result = method.invoke(object, Qt::DirectConnection, returnArg, varArgs[0], varArgs[1],
                                varArgs[2], varArgs[3], varArgs[4], varArgs[5], varArgs[6],
                                varArgs[7], varArgs[8], varArgs[9]);
    if (!result) {
      qWarning() << "invoking" << method.name() << "failed";
    }
  }

  info->setReturnValue(returnValue);
  nodeRemoteCalls.enqueue(QVariant::fromValue(loop));
  asyncLock.lock();
  async.data = (void*)&nodeRemoteCalls;
  asyncLock.unlock();
  uv_async_send(&async);
}

Helper::Helper() : d(new HelperPrivate(this)), m_isStopped(false) {
  qRegisterMetaType<GetRequestResponse*>();
  qRegisterMetaType<std::shared_ptr<UvEventLoop>>();
  qRegisterMetaType<std::shared_ptr<InvokeMethodInfo>>();
}

HelperThread::HelperThread(QObject* parent) : QThread(parent) {}

void HelperThread::run() {
  // convert QStringList to char*[]
  const QStringList& argsStrings = helperArgs();
  int size = 0;
  for (const auto& arg : argsStrings) {
    size += arg.size() + 1;
  }

  char** argv = (char**)malloc(sizeof(char*) * (argsStrings.size() + 1));  // +1 for last nullptr
  // argv needs to be contiguous
  char* s = (char*)malloc(sizeof(char) * size);
  if (argv == nullptr) {
    qWarning() << "failed to allocate memory for argument";
    exit(1);
  }

  int i = 0;
  for (i = 0; i < argsStrings.size(); i++) {
    size = argsStrings[i].size() + 1;  // +1 for 0 terminated string
    memcpy(s, argsStrings[i].toLocal8Bit().data(), size);
    argv[i] = s;
    s += size;
  }
  argv[i] = nullptr;
  int argc = argsStrings.size();
  node::Start(argc, argv);

  free(argv[0]);
  free(argv);
}

void Helper::emitSignal(const QString& str) {
  qDebug() << "emitSignal(QString)";
  QVariantList args{QVariant::fromValue(str)};
  emitSignalInternal(args);
}

void UvEventLoop::exec() {
  // FIXME: known issue with net.createConnection on Windows
  // https://github.com/abbr/deasync/issues/40
  while (!m_isFinished) {
    uv_run(uv_default_loop(), UV_RUN_ONCE);
  }
}
