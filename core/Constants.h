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

  static const QString defaultFontFamily;
  static const QString defaultUIFontFamily;

  static const int defaultFontSize;
  static const int defaultUIFontSize;

  static const char* RUN_AS_NODE;

  static const QString silkHomeDirName;

  QStringList configPaths();
  QStringList userKeymapPaths();
  QString userConfigPath();
  QString userKeymapPath();
  QString userPackagesRootDirPath() const;
  QString nodePath();
  QString npmCliPath();
  QString translationDirPath();
  QString jsLibDir();
  QString silkHomePath() const;
  QString recentOpenHistoryPath();
  QString appStatePath();
  QStringList themePaths();
  QStringList packagesPaths();
  QString userRootPackageJsonPath() const;
  QString userPackagesNodeModulesPath() const;
  QString defaultPackagePath();

private:
  friend class core::Singleton<Constants>;
  Constants() = default;

  QStringList dataDirectoryPaths();
};

}  // namespace core
