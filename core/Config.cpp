﻿#include <string>
#include <QDebug>
#include <QVariant>

#include "Config.h"
#include "ThemeProvider.h"
#include "Util.h"

namespace {
const QString END_OF_LINE_STR_KEY = "end_of_line_str";
const QString END_OF_FILE_STR_KEY = "end_of_file_str";
const QString THEME_KEY = "theme";
const QString FONT_FAMILY_KEY = "font_family";
const QString FONT_SIZE_KEY = "font_size";
const QString INDENT_USING_SPACES_KEY = "indent_using_spaces";
const QString TAB_WIDTH_KEY = "tab_width";
const QString LOCALE_KEY = "locale";
const QString SHOW_INVISIBLES_KEY = "show_invisibles";

QHash<QString, QVariant::Type> keyTypeHash;

void initKeyTypeHash() {
  keyTypeHash[END_OF_LINE_STR_KEY] = QVariant::String;
  keyTypeHash[END_OF_FILE_STR_KEY] = QVariant::String;
  keyTypeHash[THEME_KEY] = QVariant::String;
  keyTypeHash[FONT_FAMILY_KEY] = QVariant::String;
  keyTypeHash[FONT_SIZE_KEY] = QVariant::Int;
  keyTypeHash[INDENT_USING_SPACES_KEY] = QVariant::Bool;
  keyTypeHash[TAB_WIDTH_KEY] = QVariant::Int;
  keyTypeHash[LOCALE_KEY] = QVariant::String;
  keyTypeHash[SHOW_INVISIBLES_KEY] = QVariant::Bool;
}
}

namespace core {

void Config::setTheme(Theme* theme) {
  if (m_theme != theme) {
    m_theme = theme;
    m_scalarConfigs[THEME_KEY] = QVariant(theme->name);
    save(THEME_KEY, theme->name);
    emit themeChanged(theme);
  }
}

void Config::setFont(const QFont& font) {
  if (m_font != font) {
    m_font = font;
    // http://doc.qt.io/qt-5.5/qfont.html#HintingPreference-enum
    // On Windows, FullHinting makes some fonts ugly
    // On Mac, hintingPreference is ignored.
    m_font.setHintingPreference(QFont::PreferVerticalHinting);
    m_scalarConfigs[FONT_FAMILY_KEY] = QVariant(font.family());
    m_scalarConfigs[FONT_SIZE_KEY] = QVariant(font.pointSize());
    save(FONT_FAMILY_KEY, font.family());
    save(FONT_SIZE_KEY, font.pointSize());
    emit fontChanged(m_font);
  }
}

int Config::tabWidth() {
  return value(TAB_WIDTH_KEY, 4);
}

void Config::setTabWidth(int tabWidth) {
  if (setValue(TAB_WIDTH_KEY, tabWidth)) {
    emit tabWidthChanged(tabWidth);
  }
}

bool Config::indentUsingSpaces() {
  return value(INDENT_USING_SPACES_KEY, false);
}

void Config::setIndentUsingSpaces(bool value) {
  if (setValue(INDENT_USING_SPACES_KEY, value)) {
    emit indentUsingSpacesChanged(value);
  }
}

void Config::init() {
  m_mapConfigs.clear();
  m_scalarConfigs.clear();

  initKeyTypeHash();

  load();

  setTheme(ThemeProvider::theme(themeName()));
  QFont font(fontFamily(), fontSize());
  setFont(font);
}

std::unordered_map<std::string, std::string> Config::mapValue(const QString& key) {
  if (m_mapConfigs.count(key) != 0) {
    return m_mapConfigs[key];
  }

  return std::unordered_map<std::string, std::string>();
}

bool Config::contains(const QString& key) {
  return m_scalarConfigs.count(key) != 0;
}

QString Config::endOfLineStr() {
  return value(END_OF_LINE_STR_KEY, u8"\u00AC");  // U+00AC is '¬'
}

void Config::setEndOfLineStr(const QString& newValue) {
  if (setValue(END_OF_LINE_STR_KEY, newValue)) {
    emit endOfLineStrChanged(newValue);
  }
}

QString Config::endOfFileStr() {
  return value(END_OF_FILE_STR_KEY, "");
}

bool Config::enableMnemonic() {
  return value("enable_mnemonic", false);
}

QString Config::locale() {
  const QString& systemLocale = QLocale::system().name();
  const QString& locale = value(LOCALE_KEY, systemLocale);
  if (locale == "system") {
    return systemLocale;
  }
  return locale;
}

void Config::setLocale(const QString& newValue) {
  setValue(LOCALE_KEY, newValue);
}

bool Config::showInvisibles() {
  return value(SHOW_INVISIBLES_KEY, false);
}

void Config::setShowInvisibles(bool newValue) {
  if (setValue(SHOW_INVISIBLES_KEY, newValue)) {
    emit showInvisiblesChanged(newValue);
  }
}

Config::Config() : m_theme(nullptr) {}

void Config::load() {
  QStringList existingConfigPaths;
  foreach (const QString& path, Constants::configPaths()) {
    if (QFile(path).exists()) {
      existingConfigPaths.append(path);
    }
  }

  foreach (const QString& path, existingConfigPaths) { load(path); }
}

void Config::load(const QString& filename) {
  qDebug("loading configuration");

  if (!QFile(filename).exists())
    return;

  std::string name = filename.toUtf8().constData();
  try {
    YAML::Node rootNode = YAML::LoadFile(name);

    if (!rootNode.IsMap()) {
      throw std::runtime_error("root node must be a map");
    }

    for (auto it = rootNode.begin(); it != rootNode.end(); ++it) {
      QString key = QString::fromUtf8(it->first.as<std::string>().c_str()).trimmed();
      if (it->second.IsScalar()) {
        // If key is predefined, save it as an appropriate type.
        if (keyTypeHash.contains(key)) {
          switch (keyTypeHash[key]) {
            case QVariant::Bool:
              m_scalarConfigs[key] = QVariant(it->second.as<bool>());
              break;
            case QVariant::Int:
              m_scalarConfigs[key] = QVariant(it->second.as<int>());
              break;
            case QVariant::String: {
              QString value = QString::fromUtf8(it->second.as<std::string>().c_str()).trimmed();
              m_scalarConfigs[key] = value;
              break;
            }
            default:
              qWarning("Invalid type %d. key: %s", keyTypeHash[key], qPrintable(key));
              break;
          }
          // If key is defined in a package, save it as string.
        } else {
          QString value = QString::fromUtf8(it->second.as<std::string>().c_str()).trimmed();
          m_scalarConfigs[key] = value;
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
        m_mapConfigs[key] = map;
      }
    }
  } catch (const std::exception& e) {
    qWarning() << "can't load yaml file:" << filename << ", reason: " << e.what();
  } catch (...) {
    qWarning() << "can't load yaml file because of an unexpected exception: " << filename;
  }
}

QString Config::themeName() {
  return value(THEME_KEY, "Solarized (dark)");
}

QString Config::fontFamily() {
  return value(FONT_FAMILY_KEY, Constants::defaultFontFamily);
}

int Config::fontSize() {
  return value(FONT_SIZE_KEY, Constants::defaultFontSize);
}

}  // namespace core