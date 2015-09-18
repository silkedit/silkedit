#pragma once

#include <QStringList>

#include "macros.h"

namespace core {

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
  static QString userConfigPath();
  static QString userKeymapPath();
  static QString pluginRunnerPath();
  static QString pluginServerSocketPath();
  static QString translationDirPath();
  static QString pluginServerDir();
  static QString silkHomePath();

 private:
  Constants() = delete;
  ~Constants() = delete;

  static QStringList dataDirectoryPaths();
};

}  // namespace core
