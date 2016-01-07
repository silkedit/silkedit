#pragma once

#include <node.h>
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

namespace core {

class Theme;

/**
 * @brief Model class for config.yml
 * We don't use QSettings to support if condition
 */
class NODE_EXTERN Config : public QObject, public Singleton<Config> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(Config)

 public:
  ~Config() = default;

  Theme* theme() { return m_theme; }
  void setTheme(Theme* theme);

  QFont font() { return m_font; }
  void setFont(const QFont& font);

  QFontMetrics fontMetrics() { return QFontMetrics(m_font); }

  int tabWidth();
  void setTabWidth(int tabWidth);

  bool indentUsingSpaces();
  void setIndentUsingSpaces(bool value);

  QString endOfLineStr();
  void setEndOfLineStr(const QString& newValue);

  QString endOfFileStr();

  bool enableMnemonic();

  QString locale();
  void setLocale(const QString& newValue);

  bool showInvisibles();
  void setShowInvisibles(bool newValue);

  void init();
  bool contains(const QString& key);

  template <typename T>
  T value(const QString& key, const T& defaultValue) {
    if (m_scalarConfigs.count(key) != 0 && m_scalarConfigs[key].canConvert<T>()) {
      return m_scalarConfigs[key].value<T>();
    }

    return defaultValue;
  }

  template <typename T>
  bool setValue(const QString& key, T value) {
    if (m_scalarConfigs.count(key) == 0 || m_scalarConfigs[key] != value) {
      m_scalarConfigs[key] = QVariant(value);
      save(key, value);
      return true;
    }
    return false;
  }

public slots:

  // specialization of QString for convenience
  QString value(const QString& key, const QString& defaultValue = "") {
    return value<QString>(key, defaultValue);
  }

 signals:
  void themeChanged(Theme* newTheme);
  void fontChanged(QFont font);
  void tabWidthChanged(int tabWidth);
  void indentUsingSpacesChanged(bool indentUsingSpaces);
  void showInvisiblesChanged(bool);
  void endOfLineStrChanged(const QString& str);

 private:
  Theme* m_theme;
  QFont m_font;
  std::unordered_map<QString, QVariant> m_scalarConfigs;
  std::unordered_map<QString, std::unordered_map<std::string, std::string>> m_mapConfigs;

  friend class Singleton<Config>;
  Config();

  std::unordered_map<std::string, std::string> mapValue(const QString& key);
  void load();
  void load(const QString& filename);
  QString themeName();
  QString fontFamily();
  int fontSize();

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
