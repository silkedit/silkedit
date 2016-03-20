#pragma once

#include <QStringList>
#include <QObject>

#include "macros.h"
#include "Singleton.h"

namespace core {

class Constants : public QObject, public core::Singleton<Constants> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(Constants)

  Q_PROPERTY(QString userRootPackageJsonPath READ userRootPackageJsonPath CONSTANT)
  Q_PROPERTY(QString userPackagesNodeModulesPath READ userPackagesNodeModulesPath CONSTANT)
  Q_PROPERTY(QString silkHomePath READ silkHomePath CONSTANT)

 public:
  ~Constants() = default;

#ifdef Q_OS_MAC
  static const QString defaultFontFamily;
#endif

#ifdef Q_OS_WIN
  static const QString defaultFontFamily;
#endif

  static const int defaultFontSize;
  static const char* RUN_AS_NODE;

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
