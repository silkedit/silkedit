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
#include "ModeContext.h"
#include "Constants.h"
#include "Util.h"
#include "util/YamlUtils.h"

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
  if (modifiers & Qt::ShiftModifier)
    keyInt += Qt::SHIFT;
  if (modifiers & Qt::AltModifier)
    keyInt += Qt::ALT;
  if (modifiers & Qt::ControlModifier)  // Cmd key on Mac
    keyInt += Qt::CTRL;
  if (modifiers & Qt::MetaModifier)  // Ctrl key on Mac
    keyInt += Qt::META;

  return std::move(QKeySequence(keyInt));
}

QString replace(QString str, const QString& regex, const QString& after) {
  QRegularExpression re(regex, QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatchIterator iter = re.globalMatch(str);
  while (iter.hasNext()) {
    QRegularExpressionMatch match = iter.next();
    str = str.replace(match.capturedStart(), match.capturedLength(), after);
  }
  return str;
}

QKeySequence toSequence(QString str) {
  QString replacedWithMetaStr = replace(str, "ctrl|control", "meta");
  QString replacedWithCtrlStr = replace(replacedWithMetaStr, "cmd|command", "ctrl");
  QString replacedWithAltStr = replace(replacedWithCtrlStr, "opt|option", "alt");
  QString replacedWithReturnStr = replace(replacedWithAltStr, "enter", "return");

  return std::move(replacedWithReturnStr);
}

CommandArgument parseArgs(const YAML::Node& argsNode) {
  std::unordered_map<QString, QVariant> args;
  for (auto argsIter = argsNode.begin(); argsIter != argsNode.end(); argsIter++) {
    QString arg = QString::fromUtf8(argsIter->first.as<std::string>().c_str());
    QString value = QString::fromUtf8(argsIter->second.as<std::string>().c_str());
    args.insert(std::make_pair(std::move(arg), QVariant(value)));
  }

  return std::move(CommandArgument(args));
}
}

void KeymapManager::load(const QString& filename) {
  clear();

  std::string name = filename.toUtf8().constData();
  try {
    YAML::Node keymaps = YAML::LoadFile(name);

    assert(keymaps.IsSequence());

    for (auto it = keymaps.begin(); it != keymaps.end(); ++it) {
      YAML::Node node = *it;
      assert(node.IsMap());

      YAML::Node contextNode = node["context"];
      std::unique_ptr<Context> context;
      if (contextNode.IsDefined()) {
        context.reset(YamlUtils::parseContext(contextNode));
        if (!context) {
          qWarning() << "can't find a context: "
                     << QString::fromUtf8(contextNode.as<std::string>().c_str());
          continue;
        }
      }

      YAML::Node keymap = node["keymap"];
      assert(keymap.IsMap());

      for (auto keymapIter = keymap.begin(); keymapIter != keymap.end(); ++keymapIter) {
        QString keyStr = QString::fromUtf8(keymapIter->first.as<std::string>().c_str());
        QKeySequence key = toSequence(keyStr);
        YAML::Node valueNode = keymapIter->second;
        switch (valueNode.Type()) {
          case YAML::NodeType::Scalar: {
            QString cmd = QString::fromUtf8(keymapIter->second.as<std::string>().c_str());
            qDebug() << "key: " << key << ", cmd: " << cmd;
            add(key, CommandEvent(cmd, std::move(context)));
            break;
          }
          case YAML::NodeType::Map: {
            std::string cmdStr = valueNode["cmd"].as<std::string>();
            QString cmd = QString::fromUtf8(cmdStr.c_str());
            qDebug() << "key: " << key << ", cmd: " << cmd;

            YAML::Node argsNode = valueNode["args"];
            if (argsNode.IsMap()) {
              assert(argsNode.IsMap());
              CommandArgument args = parseArgs(argsNode);
              add(key, CommandEvent(cmd, args, std::move(context)));
            } else {
              add(key, CommandEvent(cmd, std::move(context)));
            }

            break;
          }
          default:
            break;
        }
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
      std::find_if(m_keymaps.begin(),
                   m_keymaps.end(),
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
      if (!QFile(Constants::standardKeymapPath()).setPermissions(
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
  m_cmdShortcuts.insert(std::make_pair(cmdEvent.cmdName(), key));
  m_keymaps.insert(std::make_pair(key, std::move(cmdEvent)));
}

void KeymapManager::clear() {
  m_keymaps.clear();
  m_cmdShortcuts.clear();
}

bool KeyHandler::eventFilter(QObject*, QEvent* event) {
  if (event->type() == QEvent::KeyPress) {
    for (const auto& filter : m_keyEventFilters) {
      if (filter->keyEventFilter(static_cast<QKeyEvent*>(event))) {
        return true;
      }
    }
  }

  return false;
}

KeyHandler::KeyHandler() {
  registerKeyEventFilter(&KeymapManager::singleton());
}

void KeyHandler::registerKeyEventFilter(IKeyEventFilter* filter) {
  m_keyEventFilters.insert(filter);
}

void KeyHandler::unregisterKeyEventFilter(IKeyEventFilter* filter) {
  m_keyEventFilters.erase(filter);
}
