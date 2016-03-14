#pragma once

#include <v8.h>
#include <yaml-cpp/yaml.h>
#include <unordered_map>
#include <fstream>
#include <QColor>
#include <QFontMetrics>
#include <QFile>
#include <QString>
#include <QObject>
#include <QFont>
#include <QDebug>

#include "macros.h"
#include "Singleton.h"
#include "stlSpecialization.h"
#include "Constants.h"
#include "ConfigDefinition.h"

namespace core {

class Theme;

/**
 * @brief Model class for config.yml
 * We don't use QSettings to support if condition
 */
class Config : public QObject, public Singleton<Config> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(Config)

 public:
  static void Init(v8::Local<v8::Object> exports);

  ~Config() = default;

  Theme* theme() { return m_theme; }
  void setTheme(Theme* theme, bool noSave = false);

  QFont font() { return m_font; }
  void setFont(const QFont& font);

  QFontMetrics fontMetrics() { return QFontMetrics(m_font); }

  int tabWidth(const QString &scopeName = "");
  void setTabWidth(int tabWidth);

  bool indentUsingSpaces();
  void setIndentUsingSpaces(bool get);

  QString endOfLineStr();
  void setEndOfLineStr(const QString& newValue);

  QString endOfFileStr();

  bool enableMnemonic();

  QString locale();
  void setLocale(const QString& newValue);

  bool showInvisibles();
  void setShowInvisibles(bool newValue);

  bool showTabsAndSpaces();
  bool wordWrap();

  bool showToolbar();

  void init();
  bool contains(const QString& key);
  void addPackageConfigDefinition(const core::ConfigDefinition& def);
  QString tabWidthKey(const QString &scopeName = "");

  template <typename T>
  T get(const QString& key, const T& defaultValue) {
    if (m_scalarConfigs.count(key) != 0 && m_scalarConfigs[key].canConvert<T>()) {
      return m_scalarConfigs[key].value<T>();
    }

    return defaultValue;
  }

  // specialization of QString for convenience
  QString get(const QString& key, const QString& defaultValue) {
    return get<QString>(key, defaultValue);
  }

  void emitConfigChange(const QString& key, QVariant oldValue, QVariant newValue);

  template <typename T>
  bool setValue(const QString& key, T value) {
    if (m_scalarConfigs.count(key) == 0 || m_scalarConfigs[key] != value) {
      QVariant newValue(value);
      QVariant oldValue;
      if (m_scalarConfigs.count(key) != 0) {
        oldValue = m_scalarConfigs[key];
      }

      m_scalarConfigs[key] = newValue;
      save(key, value);
      emitConfigChange(key, oldValue, newValue);
      return true;
    }
    return false;
  }

  QVariant defaultValue(const QString& key);

 signals:
  void themeChanged(Theme* newTheme);
  void fontChanged(QFont font);
  void tabWidthChanged(int tabWidth);
  void indentUsingSpacesChanged(bool indentUsingSpaces);
  void showInvisiblesChanged(bool);
  void showTabsAndSpacesChanged(bool);
  void wordWrapChanged(bool);
  void endOfLineStrChanged(const QString& str);
  void configChanged(const QString& key, QVariant oldValue, QVariant newValue);
  void showToolBarChanged(bool visible);

 private:
  // public API accessible from JS
  static void get(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void setFont(const v8::FunctionCallbackInfo<v8::Value>& args);
  static QMap<QString, QVariant> s_defaultValueMap;

  Theme* m_theme;
  QFont m_font;
  std::unordered_map<QString, QVariant> m_scalarConfigs;
  std::unordered_map<QString, std::unordered_map<std::string, std::string>> m_mapConfigs;
  QMap<QString, core::ConfigDefinition> m_packageConfigDefinitions;

  friend class Singleton<Config>;
  Config();

  std::unordered_map<std::string, std::string> mapValue(const QString& key);
  void load();
  void load(const QString& filename);
  QString themeName();
  QString fontFamily();
  int fontSize();
  QVariant get(const QString& key);

  void save(const QString& key, QVariant newValue) {
    if (newValue.canConvert<bool>()) {
      save(key, newValue.toBool());
    } else if (newValue.canConvert<int>()) {
      save(key, newValue.toInt());
    } else if (newValue.canConvert<double>()) {
      save(key, newValue.toDouble());
    } else if (newValue.canConvert<QString>()) {
      save(key, newValue.toString());
    } else {
      qWarning() << "invalid type:" << newValue.typeName();
    }
  }

  void save(const QString& key, const QString& newValue) {
    save(key, newValue.toUtf8().constData());
  }

  /**
   * @brief save 'key: newValue' in config.yml
   * @param key
   * @param newValue
   */
  template <typename T>
  void save(const QString& key, const T& newValue) {
    QString configFilePath = Constants::singleton().userConfigPath();

    try {
      YAML::Node rootNode;
      if (QFile(configFilePath).exists()) {
        std::string name = configFilePath.toUtf8().constData();
        rootNode = YAML::LoadFile(name);
      } else {
        rootNode = YAML::Load("");
      }

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
