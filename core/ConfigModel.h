#pragma once

#include <unordered_map>
#include <QColor>

#include "macros.h"
#include "Singleton.h"
#include "stlSpecialization.h"

class QString;

namespace core {

/**
 * @brief ConfigModel represents static configurations (load from or save to a config file)
 */
class ConfigModel {
  DISABLE_COPY_AND_MOVE(ConfigModel)

  enum class TYPE { Null, Str, Map, Array };

 public:
  static void load();
  static QString strValue(const QString& key, const QString& defaultValue = "");
  static int intValue(const QString& key, int defaultValue);
  static bool boolValue(const QString& key, bool defaultValue);
  static std::unordered_map<std::string, std::string> mapValue(const QString& key);
  static bool contains(const QString& key);
  static TYPE type(const QString& key);
  static QString themeName();
  static void saveThemeName(const QString& newValue);
  static QString fontFamily();
  static int fontSize();
  static QString endOfLineStr();
  static QColor endOfLineColor();
  static QString endOfFileStr();
  static QColor endOfFileColor();
  static int tabWidth();
  static bool indentUsingSpaces();
  static bool enableMnemonic();
  static QString locale();

 private:
  ConfigModel() = delete;
  ~ConfigModel() = delete;

  static std::unordered_map<QString, QString> m_strConfigs;
  static std::unordered_map<QString, std::unordered_map<std::string, std::string>> m_mapConfigs;

  static void load(const QString& filename);

  /**
   * @brief save 'key: newValue' in config.yml
   * @param key
   * @param newValue
   */
  static void save(const QString& key, const QString& newValue);
};

}  // namespace core
