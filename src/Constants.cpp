#include <QStandardPaths>
#include <QApplication>

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

QStringList Constants::dataDirectoryPaths() {
  QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
  paths.prepend(QApplication::applicationDirPath());
  return paths;
}
