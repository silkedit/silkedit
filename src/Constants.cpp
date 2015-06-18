#include <QStandardPaths>
#include <QApplication>
#include <QDir>

#include "Constants.h"

namespace {
static QString silkHomePath =
    QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0] + "/.silk";
}

#ifdef Q_OS_MAC
const QString Constants::defaultFontFamily = "Menlo";
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

QString Constants::standardConfigPath() {
  return silkHomePath + "/config.yml";
}

QString Constants::standardKeymapPath() {
  return silkHomePath + "/keymap.yml";
}

QString Constants::pluginRunnerPath() {
  return pluginServerDir() + "/bin/node";
}

QStringList Constants::pluginRunnerArgs() {
  QStringList args;
  // add --harmony option first
  args << "--harmony";
  // first argument is main script
  args << pluginServerDir() + "/main.js";
  // second argument is a socket path
  args << pluginServerSocketPath();
  // remaining arguments are paths to be loaded in a plugin server
  args << QDir::toNativeSeparators(QApplication::applicationDirPath() + "/packages");
  args << QDir::toNativeSeparators(silkHomePath + "/packages");
  return args;
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
