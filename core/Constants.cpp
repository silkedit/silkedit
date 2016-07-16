#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QUuid>

#include "Constants.h"

namespace {
QString resourcesPath() {
#ifdef Q_OS_MAC
  return QCoreApplication::applicationDirPath() + "/../Resources";
#else
  // Qt copies qrc files in resources directory, so don't use it.
  return QCoreApplication::applicationDirPath() + "/silk_resources";
#endif
}
}

namespace core {

const char* Constants::RUN_AS_NODE = "--run-as-node";

#ifdef Q_OS_MAC
const QString Constants::defaultFontFamily = "Source Han Code JP";
#endif

#ifdef Q_OS_WIN
const QString Constants::defaultFontFamily = "Source Han Code JP R";
#endif

#ifdef Q_OS_MAC
const int Constants::defaultFontSize = 13;
#else
const int Constants::defaultFontSize = 10;
#endif

const QString Constants::silkHomeDirName = QStringLiteral(".silk");

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
  return resourcesPath() + "/npm/bin/npm-cli.js";
}

QString Constants::translationDirPath() {
#ifdef Q_OS_MAC
  return resourcesPath() + "/translations";
#elif defined Q_OS_WIN
  return QApplication::applicationDirPath() + "/translations";
#else
  return "";
#endif
}

QStringList Constants::dataDirectoryPaths() {
  QStringList paths(silkHomePath());
  paths.prepend(resourcesPath());
  return paths;
}

QString Constants::jsLibDir() {
  return resourcesPath() + "/jslib";
}

QString Constants::defaultPackagePath() {
  return jsLibDir() + "/node_modules/silkedit/node_modules/default";
}

QString Constants::silkHomePath() const {
  return QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0] + "/" + silkHomeDirName;
}

QString Constants::recentOpenHistoryPath() {
  return QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0] +
         "/recentOpenHistory.ini";
}

QString Constants::sessionPath() {
  return QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0] + "/session.ini";
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
