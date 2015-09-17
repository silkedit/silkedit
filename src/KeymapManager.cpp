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

void replace(QString& str, const QString& regex, const QString& after) {
  QRegularExpression re(regex, QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatchIterator iter = re.globalMatch(str);
  while (iter.hasNext()) {
    QRegularExpressionMatch match = iter.next();
    str = str.replace(match.capturedStart(), match.capturedLength(), after);
  }
}

QKeySequence toSequence(QString& str) {
#ifdef Q_OS_MAC
  replace(str, "ctrl|control", "meta");
  replace(str, "cmd|command", "ctrl");
  replace(str, "opt|option", "alt");
#endif

  replace(str, "enter", "return");

  return str;
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

void KeymapManager::handleImports(const YAML::Node& node) {
  YAML::Node importsNode = node["imports"];
  if (importsNode.IsDefined() && importsNode.IsSequence()) {
    for (std::size_t i = 0; i < importsNode.size(); i++) {
      QString keymapFile = QString::fromUtf8(importsNode[i].as<std::string>().c_str());
      foreach (const QString& packageDir, Constants::packagePaths()) {
        QString replacedKeymapFile = keymapFile.replace("$silk_package_dir", packageDir);
        if (QFile(replacedKeymapFile).exists()) {
          load(replacedKeymapFile);
          break;
        }
      }
    }
  }
}

void KeymapManager::handleKeymap(const std::shared_ptr<Context>& context, const YAML::Node& node) {
  YAML::Node keymap = node["keymap"];
  if (keymap.IsDefined()) {
    for (auto keymapIter = keymap.begin(); keymapIter != keymap.end(); ++keymapIter) {
      QString keyStr = QString::fromUtf8(keymapIter->first.as<std::string>().c_str());
      QKeySequence key = toSequence(keyStr);
      YAML::Node valueNode = keymapIter->second;
      switch (valueNode.Type()) {
        case YAML::NodeType::Scalar: {
          QString command = QString::fromUtf8(keymapIter->second.as<std::string>().c_str());
          qDebug() << "key: " << key << ", command: " << command;
          add(key, CommandEvent(command, context));
          break;
        }
        case YAML::NodeType::Map: {
          std::string commandStr = valueNode["command"].as<std::string>();
          QString command = QString::fromUtf8(commandStr.c_str());
          qDebug() << "key: " << key << ", command: " << command;

          YAML::Node argsNode = valueNode["args"];
          if (argsNode.IsMap()) {
            assert(argsNode.IsMap());
            CommandArgument args = parseArgs(argsNode);
            add(key, CommandEvent(command, args, context));
          } else {
            add(key, CommandEvent(command, context));
          }

          break;
        }
        default:
          break;
      }
    }
  }
}

void KeymapManager::load(const QString& filename) {
  std::string name = filename.toUtf8().constData();
  try {
    YAML::Node keymaps = YAML::LoadFile(name);

    assert(keymaps.IsSequence());

    for (auto it = keymaps.begin(); it != keymaps.end(); ++it) {
      YAML::Node node = *it;
      assert(node.IsMap());

      handleImports(node);

      YAML::Node contextNode = node["context"];
      std::shared_ptr<Context> context;
      if (contextNode.IsDefined()) {
        context.reset(YamlUtils::parseContext(contextNode));
        if (!context) {
          qWarning() << "can't find a context: "
                     << QString::fromUtf8(contextNode.as<std::string>().c_str());
          continue;
        }
      }

      handleKeymap(context, node);
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

void KeymapManager::load() {
  QStringList existingKeymapPaths;
  foreach (const QString& path, Constants::keymapPaths()) {
    if (QFile(path).exists()) {
      existingKeymapPaths.append(path);
    }
  }

  if (existingKeymapPaths.isEmpty()) {
    qDebug("copying default keymap.yml");
    if (Util::copy(":/keymap.yml", Constants::standardKeymapPath())) {
      existingKeymapPaths.append(Constants::standardKeymapPath());
      if (!QFile(Constants::standardKeymapPath())
               .setPermissions(
                   QFileDevice::Permission::ReadOwner | QFileDevice::Permission::WriteOwner |
                   QFileDevice::Permission::ReadGroup | QFileDevice::Permission::ReadOther)) {
        qWarning("failed to set permission to %s", qPrintable(Constants::standardKeymapPath()));
      }
    } else {
      qWarning("failed to copy default keymap.yml");
    }
  }

  foreach (const QString& path, existingKeymapPaths) { load(path); }
}

QKeySequence KeymapManager::findShortcut(QString cmdName) {
  auto foundIter = m_cmdShortcuts.find(cmdName);
  if (foundIter != m_cmdShortcuts.end()) {
    auto range = m_keymaps.equal_range(foundIter->second);
    for (auto it = range.first; it != range.second; it++) {
      if (!it->second.hasContext()) {
        return m_cmdShortcuts.at(cmdName);
      }
    }
  }
  return QKeySequence();
}

bool KeymapManager::keyEventFilter(QKeyEvent* event) {
  return dispatch(event);
}

void KeymapManager::add(const QKeySequence& key, CommandEvent cmdEvent) {
  auto range = m_keymaps.equal_range(key);
  for (auto it = range.first; it != range.second; it++) {
    CommandEvent& ev = it->second;
    if (cmdEvent.context() == ev.context()) {
      // Remove registered keymap if both key and context match
      if (m_cmdShortcuts.count(ev.cmdName()) != 0) {
        m_cmdShortcuts.erase(ev.cmdName());
      }
      m_keymaps.erase(it);
      break;
    }
  }

  m_cmdShortcuts[cmdEvent.cmdName()] = key;
  m_keymaps.insert(std::make_pair(key, std::move(cmdEvent)));
}

void KeymapManager::clear() {
  m_keymaps.clear();
  m_cmdShortcuts.clear();
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
