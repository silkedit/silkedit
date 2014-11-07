#include <string>
#include <QDebug>

#include "KeymapService.h"
#include "CommandEvent.h"
#include "ModeContext.h"

namespace {
std::shared_ptr<IContext> parseContext(const YAML::Node& contextNode, ViEngine* viEngine) {
  std::shared_ptr<IContext> context;
  if (contextNode) {
    QString contextStr = QString::fromUtf8(contextNode.as<std::string>().c_str());
    QStringList list = contextStr.trimmed().split(" ", QString::SkipEmptyParts);
    if (list.size() != 3) {
      qWarning() << "context must be \"key operator operand\". size: " << list.size();
    } else {
      QString key = list[0];

      // Parse operator expression
      QString opStr = list[1];
      Operator op = Operator::EQUALS;
      if (opStr == "==") {
        op = Operator::EQUALS;
      }

      QString operand = list[2];
      if (key == "mode") {
        context.reset(new ModeContext(viEngine, op, operand));
      }
    }
  }

  return context;
}
}

std::unordered_map<QString, QVariant> KeymapService::parseArgs(const YAML::Node& argsNode) {
  std::unordered_map<QString, QVariant> args;
  for (auto argsIter = argsNode.begin(); argsIter != argsNode.end(); argsIter++) {
    QString arg = QString::fromUtf8(argsIter->first.as<std::string>().c_str());
    QString value = QString::fromUtf8(argsIter->second.as<std::string>().c_str());
    args.insert(std::make_pair(std::move(arg), QVariant(value)));
  }

  return std::move(args);
}

// FIXME: Why does this need ViEngine?
void KeymapService::load(const QString& filename, ViEngine* viEngine) {
  std::string name = filename.toUtf8().constData();
  try {
    YAML::Node keymaps = YAML::LoadFile(name);

    assert(keymaps.IsSequence());

    for (auto it = keymaps.begin(); it != keymaps.end(); ++it) {
      YAML::Node node = *it;
      assert(node.IsMap());

      std::shared_ptr<IContext> context = parseContext(node["context"], viEngine);

      YAML::Node keymap = node["keymap"];
      assert(keymap.IsMap());

      for (auto keymapIter = keymap.begin(); keymapIter != keymap.end(); ++keymapIter) {
        QString key = QString::fromUtf8(keymapIter->first.as<std::string>().c_str());
        YAML::Node valueNode = keymapIter->second;
        switch (valueNode.Type()) {
          case YAML::NodeType::Scalar: {
            QString cmd = QString::fromUtf8(keymapIter->second.as<std::string>().c_str());
            qDebug() << "key: " << key << ", cmd: " << cmd;
            m_keymaps.insert(std::make_pair(key, CommandEvent(cmd, context)));
            break;
          }
          case YAML::NodeType::Map: {
            std::string cmdStr = valueNode["cmd"].as<std::string>();
            QString cmd = QString::fromUtf8(cmdStr.c_str());
            qDebug() << "key: " << key << ", cmd: " << cmd;

            YAML::Node argsNode = valueNode["args"];
            assert(argsNode.IsMap());

            std::unordered_map<QString, QVariant> args = parseArgs(argsNode);

            m_keymaps.insert(std::make_pair(key, CommandEvent(cmd, args, context)));
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
    CommandEvent& ev = m_keymaps.at(key);
    ev.execute();
  } else {
    qDebug() << "key: " << key << " is not defined.";
  }
}
