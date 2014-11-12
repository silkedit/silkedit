#include <yaml-cpp/yaml.h>
#include <string>
#include <QDebug>
#include <QString>

#include "ConfigService.h"

void ConfigService::load(const QString& filename) {
  m_configs.clear();

  std::string name = filename.toUtf8().constData();
  try {
    YAML::Node rootNode = YAML::LoadFile(name);

    assert(rootNode.IsMap());

    for (auto it = rootNode.begin(); it != rootNode.end(); ++it) {
      QString key = QString::fromUtf8(it->first.as<std::string>().c_str()).trimmed();
      QString value = QString::fromUtf8(it->second.as<std::string>().c_str()).trimmed();
      qDebug() << key << ":" << value;
      m_configs[key] = value;
    }
  } catch (const std::exception& e) {
    qWarning() << "can't load yaml file: " << filename << ", reason: " << e.what();
  } catch (...) {
    qWarning() << "can't load yaml file because of an unexpected exception: " << filename;
  }
}

bool ConfigService::isTrue(const QString &key)
{
  if (m_configs.count(key) != 0) {
    return m_configs[key] == "true";
  }

  return false;
}
