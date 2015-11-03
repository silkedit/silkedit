#include <string>
#include <QDebug>
#include <QVariant>

#include "ConfigModel.h"
#include "Util.h"

namespace {
const QString END_OF_LINE_STR = "end_of_line_str";
const QString END_OF_FILE_STR = "end_of_file_str";
const QString THEME_KEY = "theme";
const QString FONT_FAMILY_KEY = "font_family";
const QString FONT_SIZE_KEY = "font_size";
const QString INDENT_USING_SPACES_KEY = "indent_using_spaces";
const QString TAB_WIDTH_KEY = "tab_width";
const QString LOCALE_KEY = "locale";
const QString SHOW_INVISIBLES_KEY = "show_invisibles";

QHash<QString, QVariant::Type> keyTypeHash;
}

namespace core {

std::unordered_map<QString, QVariant> ConfigModel::s_scalarConfigs;
std::unordered_map<QString, std::unordered_map<std::string, std::string>> ConfigModel::s_mapConfigs;

void ConfigModel::load(const QString& filename) {
  qDebug("loading configuration");

  if (!QFile(filename).exists())
    return;

  std::string name = filename.toUtf8().constData();
  try {
    YAML::Node rootNode = YAML::LoadFile(name);

    assert(rootNode.IsMap());

    for (auto it = rootNode.begin(); it != rootNode.end(); ++it) {
      QString key = QString::fromUtf8(it->first.as<std::string>().c_str()).trimmed();
      if (it->second.IsScalar()) {
        if (keyTypeHash.contains(key)) {
          switch (keyTypeHash[key]) {
            case QVariant::Bool:
              s_scalarConfigs[key] = QVariant(it->second.as<bool>());
              break;
            case QVariant::Int:
              s_scalarConfigs[key] = QVariant(it->second.as<int>());
              break;
            case QVariant::String: {
              QString value = QString::fromUtf8(it->second.as<std::string>().c_str()).trimmed();
              s_scalarConfigs[key] = value;
              break;
            }
            default:
              qWarning("Invalid type %d. key: %s", keyTypeHash[key], qPrintable(key));
              break;
          }
        } else {
          QString value = QString::fromUtf8(it->second.as<std::string>().c_str()).trimmed();
          s_scalarConfigs[key] = value;
        }
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
        s_mapConfigs[key] = map;
      }
    }
  } catch (const std::exception& e) {
    qWarning() << "can't load yaml file:" << filename << ", reason: " << e.what();
  } catch (...) {
    qWarning() << "can't load yaml file because of an unexpected exception: " << filename;
  }
}

void ConfigModel::load() {
  s_mapConfigs.clear();
  s_scalarConfigs.clear();

  // Init keyTypeHash
  keyTypeHash[END_OF_LINE_STR] = QVariant::String;
  keyTypeHash[END_OF_FILE_STR] = QVariant::String;
  keyTypeHash[THEME_KEY] = QVariant::String;
  keyTypeHash[FONT_FAMILY_KEY] = QVariant::String;
  keyTypeHash[FONT_SIZE_KEY] = QVariant::Int;
  keyTypeHash[INDENT_USING_SPACES_KEY] = QVariant::Bool;
  keyTypeHash[TAB_WIDTH_KEY] = QVariant::Int;
  keyTypeHash[LOCALE_KEY] = QVariant::String;
  keyTypeHash[SHOW_INVISIBLES_KEY] = QVariant::Bool;

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
  if (s_scalarConfigs.count(key) != 0) {
    return s_scalarConfigs[key].toString();
  }

  return defaultValue;
}

int ConfigModel::intValue(const QString& key, int defaultValue) {
  if (s_scalarConfigs.count(key) != 0 && s_scalarConfigs[key].canConvert<int>()) {
    return s_scalarConfigs[key].toInt();
  }

  return defaultValue;
}

bool ConfigModel::boolValue(const QString& key, bool defaultValue) {
  if (s_scalarConfigs.count(key) != 0 && s_scalarConfigs[key].canConvert<bool>()) {
    return s_scalarConfigs[key].toBool();
  }

  return defaultValue;
}

std::unordered_map<std::string, std::string> ConfigModel::mapValue(const QString& key) {
  if (s_mapConfigs.count(key) != 0) {
    return s_mapConfigs[key];
  }

  return std::unordered_map<std::string, std::string>();
}

bool ConfigModel::contains(const QString& key) {
  return s_scalarConfigs.count(key) != 0;
}

QString ConfigModel::themeName() {
  return strValue(THEME_KEY, "Solarized (light)");
}

void ConfigModel::saveThemeName(const QString& newValue) {
  s_scalarConfigs[THEME_KEY] = QVariant(newValue);
  save(THEME_KEY, newValue);
}

QString ConfigModel::fontFamily() {
  return strValue(FONT_FAMILY_KEY, Constants::defaultFontFamily);
}

void ConfigModel::saveFontFamily(const QString& newValue) {
  s_scalarConfigs[FONT_FAMILY_KEY] = QVariant(newValue);
  save(FONT_FAMILY_KEY, newValue);
}

int ConfigModel::fontSize() {
  return intValue(FONT_SIZE_KEY, Constants::defaultFontSize);
}

void ConfigModel::saveFontSize(int newValue) {
  s_scalarConfigs[FONT_SIZE_KEY] = QVariant(newValue);
  save(FONT_SIZE_KEY, newValue);
}

QString ConfigModel::endOfLineStr() {
  return strValue(END_OF_LINE_STR, "¬");
}

void ConfigModel::saveEndOfLineStr(const QString& newValue) {
  s_scalarConfigs[END_OF_LINE_STR] = QVariant(newValue);
  save(END_OF_LINE_STR, newValue);
}

QString ConfigModel::endOfFileStr() {
  return strValue(END_OF_FILE_STR, "");
}

int ConfigModel::tabWidth() {
  return intValue(TAB_WIDTH_KEY, 4);
}

void ConfigModel::saveTabWidth(int newValue) {
  s_scalarConfigs[TAB_WIDTH_KEY] = QVariant(newValue);
  save(TAB_WIDTH_KEY, newValue);
}

bool ConfigModel::indentUsingSpaces() {
  return boolValue(INDENT_USING_SPACES_KEY, false);
}

void ConfigModel::saveIndentUsingSpaces(bool newValue) {
  s_scalarConfigs[INDENT_USING_SPACES_KEY] = QVariant(newValue);
  save(INDENT_USING_SPACES_KEY, newValue);
}

bool ConfigModel::enableMnemonic() {
  return boolValue("enable_mnemonic", false);
}

QString ConfigModel::locale() {
  const QString& systemLocale = QLocale::system().name();
  const QString& locale = strValue(LOCALE_KEY, systemLocale);
  if (locale == "system") {
    return systemLocale;
  }
  return locale;
}

void ConfigModel::saveLocale(const QString& newValue) {
  s_scalarConfigs[LOCALE_KEY] = QVariant(newValue);
  save(LOCALE_KEY, newValue);
}

bool ConfigModel::showInvisibles() {
  return boolValue(SHOW_INVISIBLES_KEY, false);
}

void ConfigModel::saveShowInvisibles(bool newValue) {
  s_scalarConfigs[SHOW_INVISIBLES_KEY] = QVariant(newValue);
  save(SHOW_INVISIBLES_KEY, newValue);
}

}  // namespace core
