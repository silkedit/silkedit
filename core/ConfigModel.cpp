#include <string>
#include <QDebug>
#include <QString>

#include "ConfigModel.h"
#include "Util.h"

namespace {
const QString END_OF_LINE_STR = "end_of_line_str";
const QString END_OF_FILE_STR = "end_of_file_str";
const QString THEME_KEY = "theme";
const QString FONT_FAMILY_KEY = "font_family";
const QString FONT_SIZE_KEY = "font_size";
}

namespace core {

std::unordered_map<QString, QString> ConfigModel::m_strConfigs;
std::unordered_map<QString, std::unordered_map<std::string, std::string>> ConfigModel::m_mapConfigs;

void ConfigModel::load(const QString& filename) {
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

void ConfigModel::load() {
  QStringList existingConfigPaths;
  foreach (const QString& path, Constants::configPaths()) {
    if (QFile(path).exists()) {
      existingConfigPaths.append(path);
    }
  }

  if (existingConfigPaths.isEmpty()) {
    qDebug("copying default config.yml");
    if (Util::copy(":/config.yml", Constants::userConfigPath())) {
      existingConfigPaths.append(Constants::userConfigPath());
      if (!QFile(Constants::userConfigPath())
               .setPermissions(
                   QFileDevice::Permission::ReadOwner | QFileDevice::Permission::WriteOwner |
                   QFileDevice::Permission::ReadGroup | QFileDevice::Permission::ReadOther)) {
        qWarning("failed to set permission to %s", qPrintable(Constants::userKeymapPath()));
      }
    } else {
      qDebug("failed to copy default config.yml");
    }
  }

  foreach (const QString& path, existingConfigPaths) { load(path); }
}

QString ConfigModel::strValue(const QString& key, const QString& defaultValue) {
  if (m_strConfigs.count(key) != 0) {
    return m_strConfigs[key];
  }

  return defaultValue;
}

int ConfigModel::intValue(const QString& key, int defaultValue) {
  if (m_strConfigs.count(key) != 0) {
    bool ok;
    int value = m_strConfigs[key].toInt(&ok, 10);
    return ok ? value : defaultValue;
  }

  return defaultValue;
}

bool ConfigModel::boolValue(const QString& key, bool defaultValue) {
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

std::unordered_map<std::string, std::string> ConfigModel::mapValue(const QString& key) {
  if (m_mapConfigs.count(key) != 0) {
    return m_mapConfigs[key];
  }

  return std::unordered_map<std::string, std::string>();
}

bool ConfigModel::contains(const QString& key) {
  return m_strConfigs.count(key) != 0;
}

QString ConfigModel::themeName() {
  return strValue(THEME_KEY, "Solarized (light)");
}

void ConfigModel::saveThemeName(const QString& newValue) {
  m_strConfigs[THEME_KEY] = newValue;
  save(THEME_KEY, newValue);
}

QString ConfigModel::fontFamily() {
  return strValue(FONT_FAMILY_KEY, Constants::defaultFontFamily);
}

void ConfigModel::saveFontFamily(const QString& newValue) {
  m_strConfigs[FONT_FAMILY_KEY] = newValue;
  save(FONT_FAMILY_KEY, newValue);
}

int ConfigModel::fontSize() {
  return intValue(FONT_SIZE_KEY, Constants::defaultFontSize);
}

void ConfigModel::saveFontSize(int newValue) {
  m_strConfigs[FONT_SIZE_KEY] = newValue;
  save(FONT_SIZE_KEY, newValue);
}

QString ConfigModel::endOfLineStr() {
  if (m_mapConfigs.count(END_OF_LINE_STR) != 0) {
    std::unordered_map<std::string, std::string> map = m_mapConfigs[END_OF_LINE_STR];
    if (map.count("str") != 0) {
      return QString::fromUtf8(map["str"].c_str());
    }
  }

  return "";
}

QColor ConfigModel::endOfLineColor() {
  if (m_mapConfigs.count(END_OF_LINE_STR) != 0) {
    std::unordered_map<std::string, std::string> map = m_mapConfigs[END_OF_LINE_STR];
    if (map.count("color") != 0) {
      return QColor(QString::fromUtf8(map["color"].c_str()));
    }
  }

  return QColor();
}

QString ConfigModel::endOfFileStr() {
  if (m_mapConfigs.count(END_OF_FILE_STR) != 0) {
    std::unordered_map<std::string, std::string> map = m_mapConfigs[END_OF_FILE_STR];
    if (map.count("str") != 0) {
      return QString::fromUtf8(map["str"].c_str());
    }
  }

  return "";
}

QColor ConfigModel::endOfFileColor() {
  if (m_mapConfigs.count(END_OF_FILE_STR) != 0) {
    std::unordered_map<std::string, std::string> map = m_mapConfigs[END_OF_FILE_STR];
    if (map.count("color") != 0) {
      return QColor(QString::fromUtf8(map["color"].c_str()));
    }
  }

  return QColor();
}

int ConfigModel::tabWidth() {
  return intValue("tab_width", 4);
}

bool ConfigModel::indentUsingSpaces() {
  return boolValue("indent_using_spaces", false);
}

bool ConfigModel::enableMnemonic() {
  return boolValue("enable_mnemonic", false);
}

QString ConfigModel::locale() {
  return strValue("locale", QLocale::system().name());
}

}  // namespace core
