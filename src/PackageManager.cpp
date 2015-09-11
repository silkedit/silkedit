#include <QDir>
#include <QApplication>
#include <QStandardPaths>

#include "PackageManager.h"
#include "core/LanguageParser.h"
#include "ThemeProvider.h"
#include "core/Metadata.h"
#include "core/Constants.h"

using core::Constants;
using core::LanguageProvider;
using core::Metadata;

void PackageManager::loadPackages() {
  foreach (const QString& path, Constants::packagePaths()) { loadPackages(path); }
}

void PackageManager::loadPackages(const QString& dirName) {
  QDir dir(dirName);
  if (!dir.exists())
    return;

  QStringList filters;
  const QString tmLanguage = ".tmLanguage";
  const QString tmTheme = ".tmTheme";
  const QString tmPreferences = ".tmPreferences";
  filters << QString("*%1").arg(tmLanguage) << QString("*%1").arg(tmTheme)
          << QString("*%1").arg(tmPreferences);
  foreach (const QString& fileName, dir.entryList(filters)) {
    qDebug("loading %s", qPrintable(dir.filePath(fileName)));
    if (fileName.endsWith(tmLanguage)) {
      LanguageProvider::loadLanguage(dir.filePath(fileName));
    } else if (fileName.endsWith(tmTheme)) {
      ThemeProvider::loadTheme(dir.filePath(fileName));
    } else if (fileName.endsWith(tmPreferences)) {
      Metadata::load(dir.filePath(fileName));
    }
  }

  // find in sub directories
  foreach (const QString& subdir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
    qDebug("find sub directory: %s", qPrintable(subdir));
    loadPackages(dir.filePath(subdir));
  }
}
