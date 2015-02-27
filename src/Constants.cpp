#include <QStandardPaths>
#include <QApplication>
#include <QDir>

#include "Constants.h"

namespace {
static QString silkHomePath =
    QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0] + "/.silk";
}

QStringList Constants::configPaths() {
  QStringList configPaths;

  foreach(const QString & path, dataDirectoryPaths()) {
    configPaths.append(path + "/config.yml");
    configPaths.append(path + "/config.yaml");
  }

  return configPaths;
}

QStringList Constants::keymapPaths() {
  QStringList configPaths;

  foreach(const QString & path, dataDirectoryPaths()) {
    configPaths.append(path + "/keymap.yml");
    configPaths.append(path + "/keymap.yaml");
  }

  return configPaths;
}

QStringList Constants::packagePaths() {
  QStringList packagePaths;
  foreach(const QString & path, dataDirectoryPaths()) { packagePaths.append(path + "/Packages"); }
  return packagePaths;
}

QString Constants::standardConfigPath() { return silkHomePath + "/config.yml"; }

QString Constants::standardKeymapPath() { return silkHomePath + "/keymap.yml"; }

QString Constants::pluginRunnerPath() { return pluginServerDir() + "/bin/node"; }

QStringList Constants::pluginRunnerArgs() {
  return QStringList() << pluginServerDir() + "/main.js" << pluginServerSocketPath();
}

QString Constants::pluginServerSocketPath() {
#ifndef Q_OS_WIN32
  return QDir::tempPath() + "/silkedit.sock";
#else
  return R"(\\.\pipe\silkedit)";
#endif
}

QStringList Constants::dataDirectoryPaths() {
  QStringList paths(silkHomePath);
  paths.prepend(QApplication::applicationDirPath());
  return paths;
}

QString Constants::pluginServerDir() {
  return QApplication::applicationDirPath() + "/plugin_runner";
}
