#pragma once

#include <QStringList>

#include "macros.h"

class Constants {
  DISABLE_COPY_AND_MOVE(Constants)

 public:
#ifdef Q_OS_MAC
  static const QString defaultFontFamily;
#endif

#ifdef Q_OS_WIN
  static const QString defaultFontFamily;
#endif

  static const int defaultFontSize;

  static QStringList configPaths();
  static QStringList keymapPaths();
  static QStringList packagePaths();
  static QString standardConfigPath();
  static QString standardKeymapPath();
  static QString pluginRunnerPath();
  static QStringList pluginRunnerArgs();
  static QString pluginServerSocketPath();
  static QString translationDirPath();

 private:
  Constants() = delete;
  ~Constants() = delete;

  static QStringList dataDirectoryPaths();
  static QString pluginServerDir();
};
