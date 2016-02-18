#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QUuid>

#include "Constants.h"

namespace core {

const char* Constants::RUN_AS_NODE = "--run-as-node";

#ifdef Q_OS_MAC
const QString Constants::defaultFontFamily = "Source Han Code JP";
#endif

#ifdef Q_OS_WIN
const QString Constants::defaultFontFamily = "Consolas";
#endif

#ifdef Q_OS_MAC
const int Constants::defaultFontSize = 13;
#else
const int Constants::defaultFontSize = 12;
#endif

QStringList Constants::configPaths() {
  QStringList configPaths;

  foreach (const QString& path, dataDirectoryPaths()) { configPaths.append(path + "/config.yml"); }

  return configPaths;
}

QStringList Constants::userKeymapPaths() {
  QStringList configPaths;

  foreach (const QString& path, dataDirectoryPaths()) { configPaths.append(path + "/keymap.yml"); }

  return configPaths;
}

QString Constants::userConfigPath() {
  return silkHomePath() + "/config.yml";
}

QString Constants::userKeymapPath() {
  return silkHomePath() + "/keymap.yml";
}

QString Constants::userPackagesRootDirPath() const {
  return silkHomePath() + "/packages";
}

QString Constants::userPackagesNodeModulesPath() const {
  return userPackagesRootDirPath() + "/node_modules";
}

QString Constants::userRootPackageJsonPath() const {
  return userPackagesRootDirPath() + "/package.json";
}

// To run SilkEdit as Node.js, pass --run-as-node
// To run npm, pass bin/npm-cli.js as first argument of node.
QString Constants::nodePath() {
  return QApplication::applicationFilePath();
}

QString Constants::npmCliPath() {
  return QApplication::applicationDirPath() + "/npm/bin/npm-cli.js";
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

QString Constants::jsLibDir() {
  return QApplication::applicationDirPath() + "/jslib";
}

QString Constants::silkHomePath() const {
  return QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0] + "/.silk";
}

QString Constants::recentOpenHistoryPath() {
  return QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0] + "/recentOpenHistory.ini";
}
	
QString Constants::tabViewInformationPath() {
  return QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0] + "/tabViewInformation.ini";
}

QStringList Constants::themePaths() {
  QStringList themePaths;
  foreach (const QString& path, dataDirectoryPaths()) { themePaths.append(path + "/themes"); }
  return themePaths;
}

QStringList Constants::packagesPaths() {
  QStringList packagesPaths;
  foreach (const QString& path, dataDirectoryPaths()) { packagesPaths.append(path + "/packages"); }
  return packagesPaths;
}

}  // namespace core
