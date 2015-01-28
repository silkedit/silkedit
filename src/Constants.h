#pragma once

#include <QStringList>

#include "macros.h"

class Constants {
  DISABLE_COPY_AND_MOVE(Constants)

 public:
  static QStringList configPaths();
  static QStringList keymapPaths();
  static QStringList packagePaths();
  static QString standardConfigPath();
  static QString standardKeymapPath();
  static QString pluginRunnerPath();
  static QStringList pluginRunnerArgs();
  static QString pluginServerSocketPath();

 private:
  Constants() = delete;
  ~Constants() = delete;

  static QStringList dataDirectoryPaths();
  static QString pluginServerDir();
};
