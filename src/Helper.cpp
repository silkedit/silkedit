﻿#include <memory>
#include <QKeyEvent>
#include <QApplication>
#include <QMetaMethod>
#include <QEventLoop>
#include <QVariantMap>
#include <QDir>
#include <QThread>

#include "Helper_p.h"
#include "CommandManager.h"
#include "KeymapManager.h"
#include "SilkApp.h"
#include "atom/node_bindings.h"
#include "silkedit_node/custom_node.h"
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
  silkedit_node::Cleanup(q->m_nodeBindings->uv_env());
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

  silkedit_node::Start(argc, argv, q->m_nodeBindings);

  free(argv[0]);
  free(argv);
}

void HelperPrivate::init() {
  TextEditViewKeyHandler::singleton().registerKeyEventFilter(this);
  CommandManager::singleton().addEventFilter(std::bind(
      &HelperPrivate::cmdEventFilter, this, std::placeholders::_1, std::placeholders::_2));

  startNodeEventLoop();
}

HelperPrivate::HelperPrivate(Helper* q_ptr) : q(q_ptr) {}

CommandEventFilterResult HelperPrivate::cmdEventFilter(const std::string& name,
                                                       const CommandArgument& arg) {
  const QVariantList& args = QVariantList{QVariant::fromValue(name), QVariant::fromValue(arg)};

  QVariantList varResults =
      q->m_nodeBindings->callFunc<QVariantList>("cmdEventFilter", args, QVariantList());
  if (varResults.size() == 3 && varResults[0].canConvert<bool>() &&
      varResults[1].canConvert<QString>() && varResults[2].canConvert<QVariantMap>()) {
    return std::make_tuple(varResults[0].toBool(), varResults[1].toString().toUtf8().constData(),
                           toCommandArgument(varResults[2].value<QVariantMap>()));
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

  const QVariantList& args = QVariantList{QVariant::fromValue(type),
                                          QVariant::fromValue(key),
                                          QVariant::fromValue(event->isAutoRepeat()),
                                          QVariant::fromValue(altKey),
                                          QVariant::fromValue(ctrlKey),
                                          QVariant::fromValue(metaKey),
                                          QVariant::fromValue(shiftKey)};
  return q->m_nodeBindings->callFunc<bool>("keyEventFilter", args, false);
}

Helper::~Helper() {
  qDebug("~Helper");
}

void Helper::init() {
  d->init();
}

void Helper::sendFocusChangedEvent(const QString& viewType) {
  const QVariantList& args = QVariantList{QVariant::fromValue(viewType)};
  m_nodeBindings->callFunc("focusChanged", args);
}

void Helper::sendCommandEvent(const QString& command, const CommandArgument& cmdArgs) {
  const QVariantList& args =
      QVariantList{QVariant::fromValue(command), QVariant::fromValue(cmdArgs)};
  m_nodeBindings->callFunc("commandEvent", args);
}

void Helper::runCommand(const QString& cmd, const CommandArgument& cmdArgs) {
  const QVariantList& args = QVariantList{QVariant::fromValue(cmd), QVariant::fromValue(cmdArgs)};
  m_nodeBindings->callFunc("runCommand", args);
}

bool Helper::askCondition(const QString& name, core::Operator op, const QString& value) {
  const QVariantList& args = QVariantList{QVariant::fromValue(name),
                                          QVariant::fromValue(core::ICondition::operatorString(op)),
                                          QVariant::fromValue(value)};
  return m_nodeBindings->callFunc<bool>("askCondition", args, false);
}

QString Helper::translate(const QString& key, const QString& defaultValue) {
  const QVariantList& args =
      QVariantList{QVariant::fromValue(key), QVariant::fromValue(defaultValue)};
  return m_nodeBindings->callFunc<QString>("translate", args, defaultValue);
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

  m_nodeBindings->emitSignal(obj, method.name(), args);
}

void Helper::emitSignal() {
  emitSignalInternal(QVariantList());
}

void Helper::loadPackage(const QString& pkgName) {
  const QString& pkgDirPath =
      Constants::singleton().userPackagesNodeModulesPath() + QDir::separator() + pkgName;
  const QVariantList& args = QVariantList{QVariant::fromValue(pkgDirPath)};
  m_nodeBindings->callFunc("loadPackage", args);
}

bool Helper::removePackage(const QString& pkgName) {
  const QString& pkgDirPath =
      Constants::singleton().userPackagesNodeModulesPath() + QDir::separator() + pkgName;
  std::unique_ptr<BoolResponse> response = std::make_unique<BoolResponse>();
  const QVariantList& args =
      QVariantList{QVariant::fromValue(pkgDirPath), QVariant::fromValue(response.get())};
  QEventLoop loop;
  connect(response.get(), &BoolResponse::finished, &loop, &QEventLoop::quit);
  m_nodeBindings->callFunc("removePackage", args);
  loop.exec(QEventLoop::ExcludeUserInputEvents);
  return response->result();
}

GetRequestResponse* Helper::sendGetRequest(const QString& url, int timeoutInMs) {
  GetRequestResponse* response = new GetRequestResponse();
  const QVariantList& args = QVariantList{
      QVariant::fromValue(url), QVariant::fromValue(timeoutInMs), QVariant::fromValue(response)};
  m_nodeBindings->callFunc("sendGetRequest", args);
  return response;
}

void Helper::reloadKeymaps() {
  m_nodeBindings->callFunc("reloadKeymaps");
}

void Helper::quitApplication() {
  qDebug() << "quitApplication";
  SilkApp::instance()->quit();
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
