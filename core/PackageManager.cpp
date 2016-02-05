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

using core::LanguageProvider;
using core::Metadata;

namespace {
const QStringList tmLanguageFilter{QStringLiteral("*.tmLanguage"), QStringLiteral("*.plist")};
const QStringList tmPreferencesFilter{QStringLiteral("*.tmPreferences"), QStringLiteral("*.plist")};

void loadGrammers(const QString& path) {
  QDir dir(path);
  if (!dir.exists())
    return;

  foreach (const QString& fileName, dir.entryList(tmLanguageFilter)) {
    LanguageProvider::loadLanguage(dir.filePath(fileName));
  }
}

void loadPreferences(const QString& path) {
  QDir dir(path);
  if (!dir.exists())
    return;

  foreach (const QString& fileName, dir.entryList(tmPreferencesFilter)) {
    Metadata::load(dir.filePath(fileName));
  }
}
}

namespace core {

void PackageManager::loadFiles() {
  for (const QString& path : Constants::singleton().packagesPaths()) {
    QFile packagesJson(path + "/packages.json");
    if (!packagesJson.open(QIODevice::ReadOnly | QIODevice::Text)) {
      //    qDebug() << "Can't open" << dirName + "/packages.json";
      return;
    }

    if (const auto& packages = loadPackagesJson(packagesJson.readAll())) {
      for (const Package& pkg : *packages) {
        loadGrammers(path + "/node_modules/" + pkg.name + "/grammers");
        loadPreferences(path + "/node_modules/" + pkg.name + "/preferences");
      }
    }
  }
}

PackageManager::PackageManager() {}

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
