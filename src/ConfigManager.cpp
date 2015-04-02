#include <yaml-cpp/yaml.h>
#include <string>
#include <QDebug>
#include <QString>
#include <QFile>

#include "ConfigManager.h"
#include "Constants.h"
#include "Util.h"

std::unordered_map<QString, QString> ConfigManager::m_configs;

void ConfigManager::load(const QString& filename) {
  qDebug("loading configuration");

  if (!QFile(filename).exists())
    return;

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

void ConfigManager::load() {
  QStringList existingConfigPaths;
  foreach (const QString& path, Constants::configPaths()) {
    if (QFile(path).exists()) {
      existingConfigPaths.append(path);
    }
  }

  if (existingConfigPaths.isEmpty()) {
    qDebug("copying default config.yml");
    if (Util::copy(":/config.yml", Constants::standardConfigPath())) {
      existingConfigPaths.append(Constants::standardConfigPath());
      if (!QFile(Constants::standardConfigPath()).setPermissions(
              QFileDevice::Permission::ReadOwner | QFileDevice::Permission::WriteOwner |
              QFileDevice::Permission::ReadGroup | QFileDevice::Permission::ReadOther)) {
        qWarning("failed to set permission to %s", qPrintable(Constants::standardKeymapPath()));
      }
    } else {
      qDebug("failed to copy default config.yml");
    }
  }

  foreach (const QString& path, existingConfigPaths) { load(path); }
}

bool ConfigManager::isTrue(const QString& key) {
  if (m_configs.count(key) != 0) {
    return m_configs[key] == "true";
  }

  return false;
}

QString ConfigManager::value(const QString& key, const QString& defaultValue) {
  if (m_configs.count(key) != 0) {
    return m_configs[key];
  }

  return defaultValue;
}

bool ConfigManager::contains(const QString& key) {
  return m_configs.count(key) != 0;
}

QString ConfigManager::theme() {
  return value("theme", "Solarized (light)");
}
