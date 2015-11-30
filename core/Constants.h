#pragma once

#include <QStringList>
#include <QObject>

#include "macros.h"
#include "Singleton.h"

namespace core {

class Constants : public QObject, public core::Singleton<Constants> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(Constants)

 public:
  ~Constants() = default;

#ifdef Q_OS_MAC
  static const QString defaultFontFamily;
#endif

#ifdef Q_OS_WIN
  static const QString defaultFontFamily;
#endif

  static const int defaultFontSize;

  QStringList configPaths();
  QStringList userKeymapPaths();
  QString userConfigPath();
  QString userKeymapPath();
  QString userPackagesRootPath();
  QString userNodeModulesPath();
  Q_INVOKABLE QString userPackagesJsonPath();
  QString helperPath();
  QString npmPath();
  QString helperSocketPath();
  QString translationDirPath();
  QString helperDir();
  QString silkHomePath();
  QString recentOpenHistoryPath();
  QString tabViewInformationPath();
  QStringList themePaths();
  QStringList packagesPaths();

 private:
  friend class core::Singleton<Constants>;
  Constants() = default;

  QStringList dataDirectoryPaths();
};

}  // namespace core
