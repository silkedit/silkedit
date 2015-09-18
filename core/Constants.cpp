#include <QStandardPaths>
#include <QApplication>
#include <QDir>

#include "Constants.h"

namespace core {

#ifdef Q_OS_MAC
const QString Constants::defaultFontFamily = "Source Han Code JP";
#endif

#ifdef Q_OS_WIN
const QString Constants::defaultFontFamily = "Consolas";
#endif

const int Constants::defaultFontSize = 12;

QStringList Constants::configPaths() {
  QStringList configPaths;

  foreach (const QString& path, dataDirectoryPaths()) {
    configPaths.append(path + "/config.yml");
    configPaths.append(path + "/config.yaml");
  }

  return configPaths;
}

QStringList Constants::keymapPaths() {
  QStringList configPaths;

  foreach (const QString& path, dataDirectoryPaths()) {
    configPaths.append(path + "/keymap.yml");
    configPaths.append(path + "/keymap.yaml");
  }

  return configPaths;
}

QStringList Constants::packagePaths() {
  QStringList packagePaths;
  foreach (const QString& path, dataDirectoryPaths()) { packagePaths.append(path + "/packages"); }
  return packagePaths;
}

QString Constants::userConfigPath() {
  return silkHomePath() + "/config.yml";
}

QString Constants::userKeymapPath() {
  return silkHomePath() + "/keymap.yml";
}

QString Constants::pluginRunnerPath() {
  return pluginServerDir() + "/bin/node";
}

QString Constants::pluginServerSocketPath() {
#ifndef Q_OS_WIN32
  return QDir::tempPath() + "/silkedit.sock";
#else
  return R"(\\.\pipe\silkedit)";
#endif
}

QString Constants::translationDirPath() {
#ifdef Q_OS_MAC
  return QCoreApplication::applicationDirPath() + "/../Resources/translations";
#elif defined Q_OS_WIN
  return QApplication::applicationDirPath();
#else
  return "";
#endif
}

QStringList Constants::dataDirectoryPaths() {
  QStringList paths(silkHomePath());
  paths.prepend(QApplication::applicationDirPath());
  return paths;
}

QString Constants::pluginServerDir() {
  return QApplication::applicationDirPath() + "/plugin_runner";
}

QString Constants::silkHomePath() {
  return QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0] + "/.silk";
}

}  // namespace core
