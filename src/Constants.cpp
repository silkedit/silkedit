#include <QStandardPaths>
#include <QApplication>
#include <QDir>

#include "Constants.h"

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

QString Constants::standardConfigPath() {
  QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
  if (paths.isEmpty()) {
    qCritical("no standard data location!");
    return "";
  } else {
    return paths.at(0) + "/config.yml";
  }
}

QString Constants::standardKeymapPath() {
  QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
  if (paths.isEmpty()) {
    qCritical("no standard data location!");
    return "";
  } else {
    return paths.at(0) + "/keymap.yml";
  }
}

QString Constants::pluginServerPath() { return pluginServerDir() + "/bin/node"; }

QStringList Constants::pluginServerArgs() {
  return QStringList() << pluginServerDir() + "/main.js" << pluginServerSocketPath();
}

QString Constants::pluginServerSocketPath() {
  return QDir::tempPath() + QDir::separator() + "silk_plugin.sock";
}

QStringList Constants::dataDirectoryPaths() {
  QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
  paths.prepend(QApplication::applicationDirPath());
  return paths;
}

QString Constants::pluginServerDir() {
  return QApplication::applicationDirPath() + "/plugin_server";
}
