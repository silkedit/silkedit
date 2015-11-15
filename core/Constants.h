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
  static QStringList packagePaths();
  static QString userConfigPath();
  static QString userKeymapPath();
  static QString userPackagesDirPath();
  static QString packagesDirName();
  static QString helperPath();
  static QString npmPath();
  static QString helperSocketPath();
  static QString translationDirPath();
  static QString helperDir();
  static QString silkHomePath();
  static QString recentOpenHistoryPath();
  static QString tabViewInformationPath();
 private:
  Constants() = delete;
  ~Constants() = delete;

  static QStringList dataDirectoryPaths();
};

}  // namespace core
