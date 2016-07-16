#include <memory>
#include <QApplication>
#include <QMetaMethod>
#include <QEventLoop>
#include <QVariantMap>
#include <QDir>
#include <QThread>

#include "Helper_p.h"
#include "CommandManager.h"
#include "KeymapManager.h"
#include "App.h"
#include "core/atom/node_includes.h"
#include "core/atom/node_bindings.h"
#include "core/silkedit_node/custom_node.h"
#include "core/JSHandler.h"
#include "core/Constants.h"
#include "core/modifiers.h"
#include "core/Config.h"
#include "core/condition.h"
#include "core/Util.h"
#include "core/QVariantArgument.h"

using core::Constants;
using core::Config;
using core::Util;
using core::QVariantArgument;
using core::JSHandler;

using atom::NodeBindings;

namespace {

QStringList helperArgs() {
  QStringList args;
  args << QCoreApplication::applicationFilePath();
  // stop Node in debug mode
  // args << "--debug-brk";
  // expose global.gc()
  args << "--expose-gc";
  args << "--harmony";
  args << "--harmony_default_parameters";
  args << "--harmony_proxies";
  // first argument is main script
  args << Constants::singleton().jsLibDir() + "/main.js";
  // second argument is locale
  args << Config::singleton().locale();
  // remaining arguments are package paths
  for (const auto& path : Constants::singleton().packagesPaths()) {
    args << QDir::toNativeSeparators(path);
  }
  return args;
}
}

HelperPrivate::~HelperPrivate() {
  qDebug("~HelperPrivate");
}

void HelperPrivate::startNodeEventLoop() {
  const QStringList& argsStrings = helperArgs();
  char** argv = Util::toArgv(argsStrings);
  int argc = argsStrings.size();

  silkedit_node::Start(argc, argv, q->m_nodeBindings.get());

  free(argv[0]);
  free(argv);
}

void HelperPrivate::emitSignal(QObject* obj, const QString& signal, QVariantList args) {
  node::Environment* env = q->m_nodeBindings->uv_env();
  if (!env) {
    qDebug() << "NodeBinding is not yet initialized";
    return;
  }

  v8::Locker locker(env->isolate());
  v8::HandleScope handle_scope(env->isolate());
  v8::Context::Scope context_scope(env->context());

  return JSHandler::emitSignal(env->isolate(), obj, signal, args);
}

QVariant HelperPrivate::callFunc(const QString& funcName, QVariantList args) {
  node::Environment* env = q->m_nodeBindings->uv_env();
  if (!env) {
    qDebug() << "NodeBinding is not yet initialized";
    return QVariant();
  }

  v8::Locker locker(env->isolate());
  v8::HandleScope handle_scope(env->isolate());
  v8::Context::Scope context_scope(env->context());

  return JSHandler::callFunc(env->isolate(), funcName, args);
}

void HelperPrivate::init() {
  startNodeEventLoop();
}

HelperPrivate::HelperPrivate(Helper* q_ptr) : q(q_ptr) {}

Helper::~Helper() {
  qDebug("~Helper");
}

void Helper::init() {
  d->init();
}

void Helper::cleanup() {
  qDebug("cleanup");
  silkedit_node::Cleanup(m_nodeBindings->uv_env());
}

void Helper::runCommand(const QString& cmd, const CommandArgument& cmdArgs) {
  const QVariantList& args = QVariantList{QVariant::fromValue(cmd), QVariant::fromValue(cmdArgs)};
  d->callFunc("runCommand", args);
}

void Helper::emitSignalInternal(const QVariantList& args) {
  QObject* obj = QObject::sender();
  if (!obj) {
    qWarning() << "sender is null";
    return;
  }

  Q_ASSERT(obj->thread() == QThread::currentThread());

  const QMetaMethod& method = obj->metaObject()->method(QObject::senderSignalIndex());
  if (!method.isValid()) {
    qWarning() << "signal method is invalid";
    return;
  }

  d->emitSignal(obj, method.name(), args);
}

void Helper::emitSignal() {
  emitSignalInternal(QVariantList());
}

void Helper::loadPackage(const QString& pkgName) {
  const QString& pkgDirPath =
      Constants::singleton().userPackagesNodeModulesPath() + QDir::separator() + pkgName;
  const QVariantList& args = QVariantList{QVariant::fromValue(pkgDirPath)};
  d->callFunc("loadPackage", args);
}

bool Helper::unloadPackage(const QString& pkgName) {
  const QVariantList& args = QVariantList{QVariant::fromValue(pkgName)};
  QVariant result = d->callFunc("unloadPackage", args);
  return result.canConvert<bool>() ? result.toBool() : false;
}

GetRequestResponse* Helper::sendGetRequest(const QString& url, int timeoutInMs) {
  GetRequestResponse* response = new GetRequestResponse();
  const QVariantList& args = QVariantList{
      QVariant::fromValue(url), QVariant::fromValue(timeoutInMs), QVariant::fromValue(response)};
  d->callFunc("sendGetRequest", args);
  return response;
}

void Helper::reloadKeymaps() {
  d->callFunc("reloadKeymaps");
}

void Helper::quitApplication() {
  qDebug() << "quitApplication";
  App::instance()->quit();
}

void Helper::eval(const QString& code) {
  const QVariantList& args = QVariantList{QVariant::fromValue(code)};
  d->callFunc("eval", args);
}

void Helper::deactivatePackages() {
  d->callFunc("deactivatePackages");
}

node::Environment* Helper::uvEnv() {
  return m_nodeBindings->uv_env();
}

void Helper::uvRunOnce() {
  m_nodeBindings->UvRunOnce();
}

Helper::Helper() : d(new HelperPrivate(this)), m_nodeBindings(NodeBindings::Create()) {
  qRegisterMetaType<GetRequestResponse*>();
}

void Helper::emitSignal(const QString& str) {
  QVariantList args{QVariant::fromValue(str)};
  emitSignalInternal(args);
}

void Helper::emitSignal(int n) {
  QVariantList args{QVariant::fromValue(n)};
  emitSignalInternal(args);
}

void Helper::emitSignal(bool b) {
  QVariantList args{QVariant::fromValue(b)};
  emitSignalInternal(args);
}

void Helper::emitSignal(QWidget* old, QWidget* now) {
  qDebug() << "emitSignal(QWidget *old, QWidget *now)";
  QVariantList args{QVariant::fromValue(old), QVariant::fromValue(now)};
  emitSignalInternal(args);
}

template <typename T>
T HelperPrivate::callFunc(const QString& funcName, QVariantList args, T defaultValue) {
  node::Environment* env = q->m_nodeBindings->uv_env();
  if (!env) {
    qWarning() << "NodeBinding is not yet initialized."
               << "funcName" << funcName;
    return defaultValue;
  }
  v8::Locker locker(env->isolate());
  v8::Context::Scope context_scope(env->context());
  v8::HandleScope handle_scope(env->isolate());

  return JSHandler::callFunc(env->isolate(), funcName, args, defaultValue);
}
