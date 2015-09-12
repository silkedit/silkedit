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
#include "core/Constants.h"
#include "API.h"
#include "KeymapManager.h"
#include "CommandManager.h"
#include "InputDialog.h"
#include "core/modifiers.h"
#include "core/ConfigManager.h"

using core::Constants;
using core::ConfigManager;
using core::Operator;
using core::IContext;

PluginManager::~PluginManager() {
  qDebug("~PluginManager");
}

void PluginManager::init() {
  d->init();
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

QString PluginManager::translate(const std::string& key, const QString& defaultValue) {
  try {
    if (const boost::optional<std::string> result =
            sendRequestOption<std::tuple<std::string>, std::string>(
                "translate", std::make_tuple(key), msgpack::type::STR)) {
      return QString::fromUtf8((*result).c_str());
    } else {
      return defaultValue;
    }
  } catch (const std::exception& e) {
    qWarning() << e.what();
    return defaultValue;
  }
}

PluginManager::PluginManager() : d(new PluginManagerPrivate(this)) {
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
  d->callRequestFunc(methodName, msgId, obj);
}

void PluginManager::callNotifyFunc(const QString& methodName, const msgpack::object& obj) {
  d->callNotifyFunc(methodName, obj);
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
