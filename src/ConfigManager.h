#pragma once

#include <unordered_map>
#include <QColor>

#include "macros.h"
#include "Singleton.h"
#include "stlSpecialization.h"

class QString;

class ConfigManager {
  DISABLE_COPY_AND_MOVE(ConfigManager)

  enum class TYPE { Null, Str, Map, Array };

 public:
  static void load();
  static QString strValue(const QString& key, const QString& defaultValue = "");
  static int intValue(const QString& key, int defaultValue);
  static std::unordered_map<std::string, std::string> mapValue(const QString& key);
  static bool contains(const QString& key);
  static TYPE type(const QString& key);
  static QString theme();
  static QString fontFamily();
  static int fontSize();
  static QString endOfLineStr();
  static QColor endOfLineColor();
  static QString endOfFileStr();
  static QColor endOfFileColor();

 private:
  ConfigManager() = delete;
  ~ConfigManager() = delete;

  static std::unordered_map<QString, QString> m_strConfigs;
  static std::unordered_map<QString, std::unordered_map<std::string, std::string>> m_mapConfigs;

  static void load(const QString& filename);
};
