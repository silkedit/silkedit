#include <string>
#include <QDebug>
#include <yaml-cpp/yaml.h>

#include "keymapService.h"
#include "commandService.h"

void KeymapService::load(const QString &filename)
{
  std::string name = filename.toUtf8().constData();
  try {
    YAML::Node keymaps = YAML::LoadFile(name);

    assert(keymaps.IsSequence());

    for(YAML::const_iterator it = keymaps.begin(); it != keymaps.end(); ++it) {
      YAML::Node node = *it;
      assert(node.IsMap());
      YAML::Node keymap = node["keymap"];
      for(YAML::const_iterator keymapIter = keymap.begin(); keymapIter != keymap.end(); ++keymapIter) {
        QString key = QString::fromUtf8(keymapIter->first.as<std::string>().c_str());
        QString cmd = QString::fromUtf8(keymapIter->second.as<std::string>().c_str());
        qDebug() << "key: " << key << ", cmd: " << cmd;
        m_keymaps[key] = cmd;
      }
    }
  } catch (const std::exception& e) {
    qWarning() << "can't load yaml file: " << filename << ", reason: " << e.what();
  } catch (...) {
    qWarning() << "can't load yaml file because of an unexpected exception: " << filename;
  }
}

void KeymapService::dispatch(const QString &key)
{
  if (m_keymaps.find(key) != m_keymaps.end()) {
    CommandService::singleton().runCommand(m_keymaps[key]);
  } else {
    qDebug() << "key: " << key << " is not defined.";
  }
}
