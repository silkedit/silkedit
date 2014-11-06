#include <string>
#include <QDebug>

#include "keymapService.h"
#include "commandService.h"
#include "CommandEvent.h"

std::unordered_map<QString, QVariant> KeymapService::parseArgs(const YAML::Node& argsNode) {
  std::unordered_map<QString, QVariant> args;
  for (auto argsIter = argsNode.begin(); argsIter != argsNode.end(); argsIter++) {
    QString arg = QString::fromUtf8(argsIter->first.as<std::string>().c_str());
    QString value = QString::fromUtf8(argsIter->second.as<std::string>().c_str());
    args.insert(std::make_pair(std::move(arg), QVariant(value)));
  }

  return std::move(args);
}

void KeymapService::load(const QString& filename) {
  std::string name = filename.toUtf8().constData();
  try {
    YAML::Node keymaps = YAML::LoadFile(name);

    assert(keymaps.IsSequence());

    for (auto it = keymaps.begin(); it != keymaps.end(); ++it) {
      YAML::Node node = *it;
      assert(node.IsMap());
      YAML::Node keymap = node["keymap"];
      assert(keymap.IsMap());

      for (auto keymapIter = keymap.begin(); keymapIter != keymap.end(); ++keymapIter) {
        QString key = QString::fromUtf8(keymapIter->first.as<std::string>().c_str());
        YAML::Node valueNode = keymapIter->second;
        switch (valueNode.Type()) {
          case YAML::NodeType::Scalar: {
            QString cmd = QString::fromUtf8(keymapIter->second.as<std::string>().c_str());
            qDebug() << "key: " << key << ", cmd: " << cmd;
            m_keymaps.insert(std::make_pair(std::move(key), cmd));
            break;
          }
          case YAML::NodeType::Map: {
            std::string cmdStr = valueNode["cmd"].as<std::string>();
            QString cmd = QString::fromUtf8(cmdStr.c_str());
            YAML::Node argsNode = valueNode["args"];
            assert(argsNode.IsMap());

            std::unordered_map<QString, QVariant> args = parseArgs(argsNode);

            m_keymaps.insert(std::make_pair(std::move(key), CommandEvent(cmd, args)));
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

void KeymapService::dispatch(const QString& key) {
  if (m_keymaps.find(key) != m_keymaps.end()) {
    CommandEvent ev = m_keymaps.at(key);
    CommandService::singleton().runCommand(ev.name(), ev.args());
  } else {
    qDebug() << "key: " << key << " is not defined.";
  }
}
