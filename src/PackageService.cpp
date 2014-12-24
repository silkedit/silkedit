#include <QDir>

#include "PackageService.h"
#include "LanguageParser.h"
#include "ThemeProvider.h"

void PackageService::loadPackages()
{
  QDir dir("packages");
  Q_ASSERT(dir.exists());
  QStringList filters;
  QString tmLanguage = ".tmLanguage";
  QString tmTheme = ".tmTheme";
  filters << QString("*%1").arg(tmLanguage) << QString("*%1").arg(tmTheme);
  foreach(const QString & fileName, dir.entryList(filters)) {
    qDebug("loading %s", qPrintable(dir.filePath(fileName)));
    if (fileName.endsWith(tmLanguage)) {
      LanguageProvider::loadLanguage(dir.filePath(fileName));
    } else if (fileName.endsWith(tmTheme)) {
      ThemeProvider::loadTheme(dir.filePath(fileName));
    }
  }
}
