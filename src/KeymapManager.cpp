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
#include "util/YamlUtils.h"

using core::Constants;
using core::IKeyEventFilter;
using core::Util;
using core::AndConditionExpression;
using core::PackageManager;
using core::Package;

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
    const std::string& value = argsIter->second.as<std::string>();
    args.insert(std::make_pair(arg, value));
  }

  return args;
}
}

void KeymapManager::load(const QString& filename, const QString& source) {
  const std::string& name = filename.toUtf8().constData();
  try {
    YAML::Node keymapNode = YAML::LoadFile(name);
    if (!keymapNode.IsSequence()) {
      qWarning("keymap value must be sequence");
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
          condition = YamlUtils::parseCondition(conditionNode);
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

void KeymapManager::_dispatchFromJS(const QString &typeStr, const QString &key, bool autorep, bool altKey, bool ctrlKey, bool metaKey, bool shiftKey) {
  QEvent::Type type;
  if (typeStr == "keypress") {
    type = QEvent::KeyPress;
  } else if (typeStr == "keyup") {
    type = QEvent::KeyRelease;
  } else {
    qWarning("invalid key event type: %s", qPrintable(typeStr));
    return;
  }

  Qt::KeyboardModifiers modifiers = Qt::NoModifier;
  if (altKey) {
    modifiers |= Silk::AltModifier;
  }
  if (ctrlKey) {
    modifiers |= Silk::ControlModifier;
  }
  if (metaKey) {
    modifiers |= Silk::MetaModifier;
  }
  if (shiftKey) {
    modifiers |= Silk::ShiftModifier;
  }

  dispatch(
        new QKeyEvent(type, QKeySequence(key)[0], modifiers, key, autorep));
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

QKeySequence KeymapManager::findShortcut(QString cmdName) {
  auto foundIter = m_cmdKeymapHash.find(cmdName);
  if (foundIter != m_cmdKeymapHash.end()) {
    auto range = m_keymaps.equal_range(foundIter->second.key);
    for (auto it = range.first; it != range.second; it++) {
      // Set shortcut if command event has no condition or it has static condition and it's
      // satisfied
      if (!it->second.hasCondition()) {
        return m_cmdKeymapHash.at(cmdName).key;
      } else {
        auto condition = it->second.condition();
        if (condition->isStatic() && condition->isSatisfied()) {
          return m_cmdKeymapHash.at(cmdName).key;
        }
      }
    }
  }
  return QKeySequence();
}

bool KeymapManager::keyEventFilter(QKeyEvent* event) {
  return dispatch(event);
}

void KeymapManager::addShortcut(const QKeySequence& key, CommandEvent cmdEvent) {
  m_cmdKeymapHash.insert(std::make_pair(cmdEvent.cmdName(), Keymap{key, cmdEvent}));
  emit shortcutUpdated(cmdEvent.cmdName(), key);
}

void KeymapManager::add(const QKeySequence& key, CommandEvent cmdEvent) {
  // If cmdEvent has static condition, evaluate it immediately
  auto condition = cmdEvent.condition();
  if (condition && condition->isStatic()) {
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

  // Remove existing keymap if its priority is lower
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

  if (m_cmdKeymapHash.count(cmdEvent.cmdName()) == 0 || !cmdEvent.condition() ||
      (m_cmdKeymapHash.at(cmdEvent.cmdName()).cmd.condition() &&
       // shorter AND condition has higher priority
       // e.g. 'onMac' has higher priority than 'onMac && vim.mode == normal'
       cmdEvent.condition()->size() <
           m_cmdKeymapHash.at(cmdEvent.cmdName()).cmd.condition()->size())) {
    addShortcut(key, cmdEvent);
  }

  m_keymaps.insert(std::make_pair(key, cmdEvent));
}

TextEditViewKeyHandler::TextEditViewKeyHandler() {
  registerKeyEventFilter(&KeymapManager::singleton());
}

void TextEditViewKeyHandler::registerKeyEventFilter(IKeyEventFilter* filter) {
  m_keyEventFilters.insert(filter);
}

void TextEditViewKeyHandler::unregisterKeyEventFilter(IKeyEventFilter* filter) {
  m_keyEventFilters.erase(filter);
}

bool TextEditViewKeyHandler::dispatchKeyPressEvent(QKeyEvent* event) {
  for (const auto& filter : m_keyEventFilters) {
    if (filter->keyEventFilter(event)) {
      return true;
    }
  }

  return false;
}
