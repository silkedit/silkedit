#include <yaml-cpp/yaml.h>
#include <string>
#include <QDebug>
#include <QString>
#include <QFile>

#include "ConfigManager.h"
#include "Constants.h"
#include "Util.h"
#include "Constants.h"

namespace {
const QString END_OF_LINE_STR = "end_of_line_str";
const QString END_OF_FILE_STR = "end_of_file_str";
}

std::unordered_map<QString, QString> ConfigManager::m_strConfigs;
std::unordered_map<QString, std::unordered_map<std::string, std::string>>
    ConfigManager::m_mapConfigs;

void ConfigManager::load(const QString& filename) {
  qDebug("loading configuration");

  if (!QFile(filename).exists())
    return;

  m_strConfigs.clear();
  m_mapConfigs.clear();

  std::string name = filename.toUtf8().constData();
  try {
    YAML::Node rootNode = YAML::LoadFile(name);

    assert(rootNode.IsMap());

    for (auto it = rootNode.begin(); it != rootNode.end(); ++it) {
      QString key = QString::fromUtf8(it->first.as<std::string>().c_str()).trimmed();
      if (it->second.IsScalar()) {
        QString value = QString::fromUtf8(it->second.as<std::string>().c_str()).trimmed();
        qDebug() << key << ":" << value;
        m_strConfigs[key] = value;
      } else if (it->second.IsMap()) {
        YAML::Node mapNode = it->second;
        std::unordered_map<std::string, std::string> map;
        for (auto mapIt = mapNode.begin(); mapIt != mapNode.end(); mapIt++) {
          if (mapIt->first.IsScalar() && mapIt->second.IsScalar()) {
            std::string mapKey = mapIt->first.as<std::string>();
            std::string mapValue = mapIt->second.as<std::string>();
            map.insert(std::make_pair(mapKey, mapValue));
          }
        }
        m_mapConfigs[key] = map;
      }
    }
  } catch (const std::exception& e) {
    qWarning() << "can't load yaml file:" << filename << ", reason: " << e.what();
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
      if (!QFile(Constants::standardConfigPath())
               .setPermissions(
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

QString ConfigManager::strValue(const QString& key, const QString& defaultValue) {
  if (m_strConfigs.count(key) != 0) {
    return m_strConfigs[key];
  }

  return defaultValue;
}

int ConfigManager::intValue(const QString& key, int defaultValue) {
  if (m_strConfigs.count(key) != 0) {
    bool ok;
    int value = m_strConfigs[key].toInt(&ok, 10);
    return ok ? value : defaultValue;
  }

  return defaultValue;
}

bool ConfigManager::boolValue(const QString& key, bool defaultValue) {
  if (m_strConfigs.count(key) != 0) {
    bool value = false;
    QString valueStr = m_strConfigs[key].trimmed();
    if (valueStr == "true") {
      value = true;
    } else if (valueStr != "false") {
      qWarning("%s is not recognized as boolean value", qPrintable(valueStr));
    }
    return value;
  }

  return defaultValue;
}

std::unordered_map<std::string, std::string> ConfigManager::mapValue(const QString& key) {
  if (m_mapConfigs.count(key) != 0) {
    return m_mapConfigs[key];
  }

  return std::unordered_map<std::string, std::string>();
}

bool ConfigManager::contains(const QString& key) {
  return m_strConfigs.count(key) != 0;
}

QString ConfigManager::theme() {
  return strValue("theme", "Solarized (light)");
}

QString ConfigManager::fontFamily() {
  return strValue("font_family", Constants::defaultFontFamily);
}

int ConfigManager::fontSize() {
  return intValue("font_size", Constants::defaultFontSize);
}

QString ConfigManager::endOfLineStr() {
  if (m_mapConfigs.count(END_OF_LINE_STR) != 0) {
    std::unordered_map<std::string, std::string> map = m_mapConfigs[END_OF_LINE_STR];
    if (map.count("str") != 0) {
      return QString::fromUtf8(map["str"].c_str());
    }
  }

  return "";
}

QColor ConfigManager::endOfLineColor() {
  if (m_mapConfigs.count(END_OF_LINE_STR) != 0) {
    std::unordered_map<std::string, std::string> map = m_mapConfigs[END_OF_LINE_STR];
    if (map.count("color") != 0) {
      return QColor(QString::fromUtf8(map["color"].c_str()));
    }
  }

  return QColor();
}

QString ConfigManager::endOfFileStr() {
  if (m_mapConfigs.count(END_OF_FILE_STR) != 0) {
    std::unordered_map<std::string, std::string> map = m_mapConfigs[END_OF_FILE_STR];
    if (map.count("str") != 0) {
      return QString::fromUtf8(map["str"].c_str());
    }
  }

  return "";
}

QColor ConfigManager::endOfFileColor() {
  if (m_mapConfigs.count(END_OF_FILE_STR) != 0) {
    std::unordered_map<std::string, std::string> map = m_mapConfigs[END_OF_FILE_STR];
    if (map.count("color") != 0) {
      return QColor(QString::fromUtf8(map["color"].c_str()));
    }
  }

  return QColor();
}

int ConfigManager::tabWidth() {
  return intValue("tab_width", 4);
}

bool ConfigManager::indentUsingSpaces() {
  return boolValue("indent_using_spaces", false);
}
