#include <QDir>
#include <QApplication>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonArray>

#include "PackageManager.h"
#include "LanguageParser.h"
#include "Metadata.h"
#include "Constants.h"
#include "Package.h"

namespace {
const QString tmLanguage = ".tmLanguage";
const QString tmPreferences = ".tmPreferences";
QStringList filters{QString("*%1").arg(tmLanguage), QString("*%1").arg(tmPreferences)};
}

namespace core {

void PackageManager::loadGrammers() {
  for (const QString& path : Constants::singleton().packagesPaths()) {
    QFile packagesJson(path + "/packages.json");
    if (!packagesJson.open(QIODevice::ReadOnly | QIODevice::Text)) {
      //    qDebug() << "Can't open" << dirName + "/packages.json";
      return;
    }

    if (const auto& packages = loadPackagesJson(packagesJson.readAll())) {
      for (const Package& pkg : *packages) {
        loadGrammers(path + "/node_modules/" + pkg.name + "/grammers");
      }
    }
  }
}

PackageManager::PackageManager() {}

void PackageManager::loadGrammers(const QString& path) {
  QDir dir(path);
  if (!dir.exists())
    return;

  foreach (const QString& fileName, dir.entryList(filters)) {
    //    qDebug("loading %s", qPrintable(dir.filePath(fileName)));
    if (fileName.endsWith(tmLanguage)) {
      LanguageProvider::loadLanguage(dir.filePath(fileName));
    } else if (fileName.endsWith(tmPreferences)) {
      Metadata::load(dir.filePath(fileName));
    }
  }

  // find in sub directories
  for (const QString& subdir : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
    qDebug("find sub directory: %s", qPrintable(subdir));
    loadGrammers(dir.filePath(subdir));
  }
}

boost::optional<QList<Package>> PackageManager::loadPackagesJson(const QByteArray& json) {
  const QJsonDocument& doc = QJsonDocument::fromJson(json);
  if (doc.isNull()) {
    return boost::none;
  }

  const QJsonArray& jsonPackages = doc.array();
  QList<Package> packages;
  for (const QJsonValue& value : jsonPackages) {
    packages.append(Package::fromJson(value));
  }
  return packages;
}

}  // namespace core
