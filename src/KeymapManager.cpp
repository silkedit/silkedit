#include <boost/optional.hpp>
#include <yaml-cpp/yaml.h>
#include <string>
#include <QDebug>
#include <QShortcut>
#include <QList>
#include <QString>
#include <QRegularExpression>
#include <QKeyEvent>
#include <QApplication>

#include "KeymapManager.h"
#include "CommandEvent.h"
#include "CommandManager.h"
#include "Helper.h"
#include "core/Constants.h"
#include "core/Util.h"
#include "core/AndConditionExpression.h"
#include "core/PackageManager.h"
#include "core/Package.h"
#include "core/modifiers.h"
#include "core/V8Util.h"
#include "core/KeyEvent.h"
#include "util/YamlUtil.h"
#include "core/FunctionInfo.h"
#include "atom/node_includes.h"

using core::Constants;
using core::Util;
using core::AndConditionExpression;
using core::PackageManager;
using core::Package;
using core::V8Util;
using core::KeyEvent;
using core::FunctionInfo;

using v8::UniquePersistent;
using v8::ObjectTemplate;
using v8::EscapableHandleScope;
using v8::Local;
using v8::String;
using v8::PropertyCallbackInfo;
using v8::Value;
using v8::Isolate;
using v8::Array;
using v8::Object;
using v8::MaybeLocal;
using v8::Maybe;
using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Boolean;
using v8::Function;
using v8::Null;
using v8::FunctionTemplate;

namespace {

QKeySequence toSequence(const QKeyEvent& ev) {
  int keyInt = ev.key();
  Qt::KeyboardModifiers modifiers = ev.modifiers();
  /*
  Note: On Mac OS X, the ControlModifier value corresponds to the Command keys on the Macintosh
  keyboard, and the MetaModifier value corresponds to the Control keys. The KeypadModifier value
  will also be set when an arrow key is pressed as the arrow keys are considered part of the keypad.
  Note: On Windows Keyboards, Qt::MetaModifier and Qt::Key_Meta are mapped to the Windows key.

  http://qt-project.org/doc/qt-5.3/qt.html#KeyboardModifier-enum
  */
  if (modifiers & Qt::ShiftModifier && keyInt != Qt::Key_Shift)
    keyInt += Qt::SHIFT;
  if (modifiers & Qt::AltModifier && keyInt != Qt::Key_Alt)
    keyInt += Qt::ALT;
  if (modifiers & Qt::ControlModifier && keyInt != Qt::Key_Control)  // Cmd key on Mac
    keyInt += Qt::CTRL;
  if (modifiers & Qt::MetaModifier && keyInt != Qt::Key_Meta)  // Ctrl key on Mac
    keyInt += Qt::META;

  return QKeySequence(keyInt);
}

CommandArgument parseArgs(const YAML::Node& argsNode) {
  CommandArgument args;
  for (auto argsIter = argsNode.begin(); argsIter != argsNode.end(); argsIter++) {
    const std::string& arg = argsIter->first.as<std::string>();
    QVariant value = Util::toVariant(argsIter->second.as<std::string>());
    args.insert(std::make_pair(arg, value));
  }

  return args;
}
}

void KeymapManager::load(const QString& filename, const QString& source) {
  std::string name = filename.toUtf8().constData();
  try {
    YAML::Node keymapNode = YAML::LoadFile(name);

    if (!keymapNode.IsSequence()) {
      return;
    }

    for (std::size_t i = 0; i < keymapNode.size(); i++) {
      YAML::Node keymapDefNode = keymapNode[i];
      if (!keymapDefNode.IsMap()) {
        qWarning("keymap definition must be a map");
        continue;
      }

      YAML::Node keyNode = keymapDefNode["key"];
      if (!keyNode.IsScalar()) {
        qWarning("key is not defined");
        continue;
      }
      QString keyStr = QString::fromUtf8(keyNode.as<std::string>().c_str());
      QKeySequence key = Util::toSequence(keyStr);
      YAML::Node commandNode = keymapDefNode["command"];
      QString command = "";
      if (commandNode.IsScalar()) {
        const std::string& commandStr = commandNode.as<std::string>();
        command = QString::fromUtf8(commandStr.c_str());
      }
      //      qDebug() << "key: " << key << ", command: " << command;

      YAML::Node ifNode = keymapDefNode["if"];
      boost::optional<AndConditionExpression> condition;
      if (ifNode.IsDefined()) {
        YAML::Node conditionNode = keymapDefNode["if"];
        if (conditionNode.IsDefined()) {
          condition = YamlUtil::parseCondition(conditionNode);
          if (!condition) {
            qWarning() << "can't find a condition: "
                       << QString::fromUtf8(conditionNode.as<std::string>().c_str());
          }
        }
      }

      YAML::Node argsNode = keymapDefNode["args"];
      if (argsNode.IsMap()) {
        Q_ASSERT(argsNode.IsMap());
        CommandArgument args = parseArgs(argsNode);
        add(key,
            CommandEvent(command, args, condition, source, CommandEvent::USER_KEYMAP_PRIORITY));
      } else {
        add(key, CommandEvent(command, condition, source, CommandEvent::USER_KEYMAP_PRIORITY));
      }
    }

    emit keymapUpdated();
  } catch (const std::exception& e) {
    qWarning() << "can't load yaml file: " << filename << ", reason: " << e.what();
  } catch (...) {
    qWarning() << "can't load yaml file because of an unexpected exception: " << filename;
  }
}

bool KeymapManager::dispatch(QKeyEvent* event, int repeat) {
  QKeySequence key = toSequence(*event);
  qDebug() << "key: " << key;

  if (!m_partiallyMatchedKeyString.isEmpty()) {
    qDebug() << "partially matched key:" << m_partiallyMatchedKeyString;
    key = QKeySequence(m_partiallyMatchedKeyString + "," + key.toString());
    qDebug() << "combinded key:" << key;
  }

  // check exact match
  if (m_keymaps.find(key) != m_keymaps.end()) {
    m_partiallyMatchedKeyString.clear();
    auto range = m_keymaps.equal_range(key);
    for (auto it = range.first; it != range.second; it++) {
      CommandEvent& ev = it->second;
      if (ev.execute(repeat)) {
        return true;
      }
    }
  }

  // check partial match
  auto partiallyMatchedKey =
      std::find_if(m_keymaps.begin(), m_keymaps.end(),
                   [key](const std::unordered_map<QKeySequence, CommandEvent>::value_type& p) {
                     return key.matches(p.first) == QKeySequence::PartialMatch;
                   });

  if (partiallyMatchedKey != m_keymaps.end()) {
    qDebug("partial match");
    m_partiallyMatchedKeyString = key.toString();
    return true;
  }

  // no match
  return false;
}

void KeymapManager::_assignJSKeyEventFilter(core::FunctionInfo info) {
  Isolate* isolate = info.isolate;
  UniquePersistent<Function> perFn(isolate, info.fn);
  m_jsKeyEventFilter.Reset(info.isolate, info.fn);
}

KeymapManager::KeymapManager() {
  connect(&PackageManager::singleton(), &PackageManager::packageRemoved, this,
          [=](const Package& pkg) {
            for (auto it = m_keymaps.begin(); it != m_keymaps.end();) {
              if (it->second.source() == pkg.name) {
                it = m_keymaps.erase(it);
              } else {
                ++it;
              }
            }
            emit keymapUpdated();
          });

  connect(&CommandManager::singleton(), &CommandManager::commandRemoved, this,
          [=](const QString& name) { m_cmdKeymapHash.erase(name); });
}

void KeymapManager::removeShortcut(const QString& cmdName) {
  m_cmdKeymapHash.erase(cmdName);
  emit shortcutUpdated(cmdName, QKeySequence());
}

void KeymapManager::removeKeymap() {
  m_emptyCmdKeymap.clear();
  m_cmdKeymapHash.clear();
  m_keymaps.clear();
}

void KeymapManager::loadUserKeymap() {
  bool firstTime = m_keymaps.empty();

  removeKeymap();

  QStringList existingKeymapPaths;
  foreach (const QString& path, Constants::singleton().userKeymapPaths()) {
    if (QFile(path).exists()) {
      existingKeymapPaths.append(path);
    }
  }

  foreach (const QString& path, existingKeymapPaths) {
    load(path, CommandEvent::USER_KEYMAP_SOURCE);
  }

  // When reloading user keymap, reload package keymaps again
  if (!firstTime) {
    Helper::singleton().reloadKeymaps();
  }
}

// returns the command name which is activated when key is pressed
QString KeymapManager::findCmdName(QKeySequence key) {
  if (m_keymaps.find(key) != m_keymaps.end()) {
    auto range = m_keymaps.equal_range(key);
    for (auto it = range.first; it != range.second; it++) {
      CommandEvent& ev = it->second;
      if (!ev.condition() || ev.condition()->isSatisfied()) {
        return ev.cmdName();
      }
    }
  }

  return "";
}

QKeySequence KeymapManager::findShortcut(QString cmdName) {
  auto foundIter = m_cmdKeymapHash.find(cmdName);
  if (foundIter != m_cmdKeymapHash.end()) {
    auto range = m_cmdKeymapHash.equal_range(cmdName);
    for (auto it = range.first; it != range.second; it++) {
      if ((!it->second.cmd.condition() || it->second.cmd.condition()->isSatisfied()) &&
          findCmdName(it->second.key) == cmdName) {
        return it->second.key;
      }
    }
  }
  return QKeySequence();
}

bool KeymapManager::handle(QKeyEvent* event) {
  bool result = runJSKeyEventFilter(event);
  if (result) {
    qDebug() << "key event is handled by an event filter";
    return true;
  }

  return dispatch(event);
}

bool KeymapManager::runJSKeyEventFilter(QKeyEvent* event) {
  node::Environment* env = Helper::singleton().uvEnv();
  // run command filters
  if (!env) {
    qDebug() << "env is null";
    return false;
  }

  Isolate* isolate = env->isolate();
  v8::Locker locker(env->isolate());
  v8::Context::Scope context_scope(env->context());
  v8::HandleScope handle_scope(env->isolate());

  const int argc = 1;
  v8::Local<Value> argv[argc];
  argv[0] = V8Util::toV8ObjectFrom(isolate, new KeyEvent(event));

  QVariant handled = V8Util::callJSFunc(isolate, m_jsKeyEventFilter.Get(isolate),
                                        v8::Undefined(isolate), argc, argv);

  if (!handled.canConvert<bool>()) {
    qWarning() << "handled is not boolean";
    return false;
  }

  return handled.toBool();
}

void KeymapManager::addShortcut(const QKeySequence& key, CommandEvent cmdEvent) {
  m_cmdKeymapHash.insert(std::make_pair(cmdEvent.cmdName(), Keymap{key, cmdEvent}));
  emit shortcutUpdated(cmdEvent.cmdName(), key);
}

void KeymapManager::add(const QKeySequence& key, CommandEvent cmdEvent) {
  // If cmdEvent has static condition, evaluate it immediately
  // e.g. filter out keymaps for different os
  auto condition = cmdEvent.condition();
  if (condition && condition->hasStatic()) {
    if (!condition->isSatisfied()) {
      return;
    }
  }

  // Empty command keymap is for disabling default keymap
  // If you want to disable '- { key: esc, command: vim.command_mode, if: vim.mode == insert }',
  // you need to define this empty command keymap - { key: esc, command: , if: vim.mode ==
  // insert }
  if (cmdEvent.cmdName().isEmpty()) {
    m_emptyCmdKeymap.insert(
        std::make_pair(key, CommandEvent("", condition, CommandEvent::USER_KEYMAP_SOURCE,
                                         CommandEvent::USER_KEYMAP_PRIORITY)));
    return;
  } else if (m_emptyCmdKeymap.count(key) != 0 &&
             m_emptyCmdKeymap.at(key).condition() == condition) {
    return;
  }

  // Remove existing keymap if it has same condition and its priority is lower
  // e.g.
  // - { key: 'ctrl+`', command: show_console, if on_mac}
  // - { key: 'ctrl+`', command: hide_console, if on_mac}
  auto range = m_keymaps.equal_range(key);
  for (auto it = range.first; it != range.second; it++) {
    CommandEvent& ev = it->second;
    if (cmdEvent.condition() == ev.condition()) {
      if (ev.priority() < cmdEvent.priority()) {
        if (m_cmdKeymapHash.count(ev.cmdName()) != 0) {
          m_cmdKeymapHash.erase(ev.cmdName());
        }
        m_keymaps.erase(it);
        break;
      } else {
        // Ignore keymap defined in package keymap.yml
        return;
      }
    }
  }

  addShortcut(key, cmdEvent);

  m_keymaps.insert(std::make_pair(key, cmdEvent));
}
