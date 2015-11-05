﻿#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QUuid>

#include "Constants.h"

namespace {
#ifdef Q_OS_WIN
const QString serverSocketPath = R"(\\.\pipe\silkedit_)" + QUuid::createUuid().toString();
#else
const QString serverSocketPath = QDir::tempPath() + "/silkedit.sock";
#endif

const QString PACKAGES_NAME = "packages";
}

namespace core {

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

QStringList Constants::keymapPaths() {
  QStringList configPaths;

  foreach (const QString& path, dataDirectoryPaths()) { configPaths.append(path + "/keymap.yml"); }

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

QString Constants::userPackagesDirPath() {
  return silkHomePath() + "/" + PACKAGES_NAME;
}

QString Constants::packagesDirName() {
  return PACKAGES_NAME;
}

QString Constants::pluginRunnerPath() {
  return pluginServerDir() + "/bin/node";
}

QString Constants::npmPath() {
#ifdef Q_OS_WIN
  return pluginServerDir() + "/bin/npm.cmd";
#else
  return pluginServerDir() + "/bin/npm";
#endif
}

QString Constants::pluginServerSocketPath() {
  return serverSocketPath;
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