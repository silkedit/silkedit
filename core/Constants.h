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
  static QStringList userKeymapPaths();
  static QString userConfigPath();
  static QString userKeymapPath();
  static QString userPackagesRootPath();
  static QString userNodeModulesPath();
  static QString userPackagesJsonPath();
  static QString helperPath();
  static QString npmPath();
  static QString helperSocketPath();
  static QString translationDirPath();
  static QString helperDir();
  static QString silkHomePath();
  static QString recentOpenHistoryPath();
  static QString tabViewInformationPath();
  static QStringList themePaths();
  static QStringList packagesPaths();

 private:
  Constants() = delete;
  ~Constants() = delete;

  static QStringList dataDirectoryPaths();
};

}  // namespace core
