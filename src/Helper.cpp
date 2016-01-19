﻿#include <memory>
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
#include "atom/node_includes.h"
#include "atom/node_bindings.h"
#include "silkedit_node/custom_node.h"
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
  //  args << "--expose-gc";
  args << "--harmony";
  args << "--harmony_default_parameters";
  args << "--harmony_proxies";
  // first argument is main script
  args << Constants::singleton().jsLibDir() + "/main.js";
  // second argument is locale
  args << Config::singleton().locale();
  // remaining arguments are paths to be loaded in silkedit_helper
  args << QDir::toNativeSeparators(QApplication::applicationDirPath() + "/packages");
  args << QDir::toNativeSeparators(Constants::singleton().silkHomePath() + "/packages");
  return args;
}

}

HelperPrivate::~HelperPrivate() {
  qDebug("~HelperPrivate");
}

void HelperPrivate::startNodeEventLoop() {
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
  connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, &Helper::singleton(),
          &Helper::cleanup);
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

QString Helper::translate(const QString& key, const QString& defaultValue) {
  const QVariantList& args =
      QVariantList{QVariant::fromValue(key), QVariant::fromValue(defaultValue)};
  return d->callFunc<QString>("translate", args, defaultValue);
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

bool Helper::removePackage(const QString& pkgName) {
  const QString& pkgDirPath =
      Constants::singleton().userPackagesNodeModulesPath() + QDir::separator() + pkgName;
  std::unique_ptr<BoolResponse> response = std::make_unique<BoolResponse>();
  const QVariantList& args =
      QVariantList{QVariant::fromValue(pkgDirPath), QVariant::fromValue(response.get())};
  QEventLoop loop;
  connect(response.get(), &BoolResponse::finished, &loop, &QEventLoop::quit);
  d->callFunc("removePackage", args);
  loop.exec(QEventLoop::ExcludeUserInputEvents);
  return response->result();
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

node::Environment *Helper::uvEnv()
{
  return m_nodeBindings->uv_env();
}

void Helper::uvRunOnce() {
  m_nodeBindings->UvRunOnce();
}

Helper::Helper() : d(new HelperPrivate(this)), m_nodeBindings(NodeBindings::Create()) {
  qRegisterMetaType<GetRequestResponse*>();
}

void Helper::emitSignal(const QString& str) {
  qDebug() << "emitSignal(QString)";
  QVariantList args{QVariant::fromValue(str)};
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
    qDebug() << "NodeBinding is not yet initialized";
    return defaultValue;
  }
  v8::Locker locker(env->isolate());
  v8::Context::Scope context_scope(env->context());
  v8::HandleScope handle_scope(env->isolate());

  return JSHandler::callFunc(env->isolate(), funcName, args, defaultValue);
}
