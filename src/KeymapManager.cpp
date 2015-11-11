﻿#include <boost/optional.hpp>
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
#include "core/Constants.h"
#include "core/Util.h"
#include "util/YamlUtils.h"

using core::Constants;
using core::IKeyEventFilter;
using core::Util;

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
    std::string arg = argsIter->first.as<std::string>();
    std::string value = argsIter->second.as<std::string>();
    args.insert(std::make_pair(std::move(arg), std::move(value)));
  }

  return std::move(args);
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

      QString keyStr = QString::fromUtf8(keymapDefNode["key"].as<std::string>().c_str());
      QKeySequence key = Util::toSequence(keyStr);
      std::string commandStr = keymapDefNode["command"].as<std::string>();
      QString command = QString::fromUtf8(commandStr.c_str());
      //      qDebug() << "key: " << key << ", command: " << command;

      YAML::Node ifNode = keymapDefNode["if"];
      boost::optional<ConditionExpression> condition;
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
        assert(argsNode.IsMap());
        CommandArgument args = parseArgs(argsNode);
        add(key, CommandEvent(command, args, condition, source));
      } else {
        add(key, CommandEvent(command, condition, source));
      }
    }
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

void KeymapManager::loadUserKeymap() {
  m_cmdShortcuts.clear();
  m_keymaps.clear();

  QStringList existingKeymapPaths;
  foreach (const QString& path, Constants::userKeymapPaths()) {
    if (QFile(path).exists()) {
      existingKeymapPaths.append(path);
    }
  }

  foreach (const QString& path, existingKeymapPaths) { load(path, ""); }
}

QKeySequence KeymapManager::findShortcut(QString cmdName) {
  auto foundIter = m_cmdShortcuts.find(cmdName);
  if (foundIter != m_cmdShortcuts.end()) {
    auto range = m_keymaps.equal_range(foundIter->second);
    for (auto it = range.first; it != range.second; it++) {
      // Set shortcut if command event has no condition or it has static condition and it's
      // satisfied
      if (!it->second.hasCondition()) {
        return m_cmdShortcuts.at(cmdName);
      } else {
        auto condition = it->second.condition();
        if (condition->isStatic() && condition->isSatisfied()) {
          return m_cmdShortcuts.at(cmdName);
        }
      }
    }
  }
  return QKeySequence();
}

bool KeymapManager::keyEventFilter(QKeyEvent* event) {
  return dispatch(event);
}

void KeymapManager::add(const QKeySequence& key, CommandEvent cmdEvent) {
  // If cmdEvent has static condition, evaluate it immediately
  auto condition = cmdEvent.condition();
  if (condition && condition->isStatic()) {
    if (!condition->isSatisfied()) {
      return;
    }
  }

  auto range = m_keymaps.equal_range(key);
  for (auto it = range.first; it != range.second; it++) {
    CommandEvent& ev = it->second;
    // Ignore if both key and conditon match
    if (cmdEvent.condition() == ev.condition()) {
      return;
    }
  }

  if (!cmdEvent.hasCondition()) {
    m_cmdShortcuts[cmdEvent.cmdName()] = key;
    emit shortcutUpdated(cmdEvent.cmdName(), key);
  }

  m_keymaps.insert(std::make_pair(key, std::move(cmdEvent)));
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
