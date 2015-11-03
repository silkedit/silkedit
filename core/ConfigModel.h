#pragma once

#include <yaml-cpp/yaml.h>
#include <unordered_map>
#include <fstream>
#include <QColor>
#include <QFile>

#include "macros.h"
#include "Singleton.h"
#include "stlSpecialization.h"
#include "Constants.h"

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
  static void saveFontFamily(const QString& newValue);
  static int fontSize();
  static void saveFontSize(int newValue);
  static QString endOfLineStr();
  static QColor endOfLineColor();
  static QString endOfFileStr();
  static QColor endOfFileColor();
  static int tabWidth();
  static void saveTabWidth(int newValue);
  static bool indentUsingSpaces();
  static void saveIndentUsingSpaces(bool newValue);
  static bool enableMnemonic();
  static QString locale();
  static void saveLocale(const QString& newValue);

 private:
  ConfigModel() = delete;
  ~ConfigModel() = delete;

  static std::unordered_map<QString, QString> m_strConfigs;
  static std::unordered_map<QString, std::unordered_map<std::string, std::string>> m_mapConfigs;

  static void load(const QString& filename);

  static void save(const QString& key, const QString& newValue) {
    save(key, newValue.toUtf8().constData());
  }

  /**
   * @brief save 'key: newValue' in config.yml
   * @param key
   * @param newValue
   */
  template <typename T>
  static void save(const QString& key, const T& newValue) {
    QString configFilePath = Constants::userConfigPath();

    if (!QFile(configFilePath).exists())
      return;

    std::string name = configFilePath.toUtf8().constData();
    try {
      YAML::Node rootNode = YAML::LoadFile(name);

      assert(rootNode.IsMap());
      rootNode[key.toUtf8().constData()] = newValue;

      std::string configFileName = configFilePath.toUtf8().constData();
      std::ofstream fout(configFileName);
      fout << rootNode;  // dump it back into the file
    } catch (const std::exception& e) {
      qWarning() << "can't edit yaml file:" << configFilePath << ", reason: " << e.what();
    } catch (...) {
      qWarning() << "can't edit yaml file because of an unexpected exception: " << configFilePath;
    }
  }
};

}  // namespace core
