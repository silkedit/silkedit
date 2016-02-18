#include <QDir>
#include <QApplication>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

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
const QString PackageManager::DEPENDENCIES = QStringLiteral("dependencies");

void PackageManager::loadFiles() {
  for (const QString& path : Constants::singleton().packagesPaths()) {
    QFile rootPackageJson(path + "/package.json");
    if (!rootPackageJson.open(QIODevice::ReadOnly | QIODevice::Text)) {
      //    qDebug() << "Can't open" << dirName + "/package.json";
      continue;
    }
    const QJsonDocument& doc = QJsonDocument::fromJson(rootPackageJson.readAll());
    if (doc.isNull()) {
      continue;
    }

    const auto& rootObj = doc.object();
    if (rootObj.isEmpty() || !rootObj.contains(DEPENDENCIES) || !rootObj[DEPENDENCIES].isObject()) {
      continue;
    }

    Q_ASSERT(rootObj[DEPENDENCIES].isObject());
    for (const auto& pkg : rootObj[DEPENDENCIES].toObject().keys()) {
      loadGrammers(path + "/node_modules/" + pkg + "/grammers");
      loadPreferences(path + "/node_modules/" + pkg + "/preferences");
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

bool PackageManager::_ensureRootPackageJson()
{
  // ensure that user packgaes directory exists
  if (!QFileInfo::exists(Constants::singleton().userPackagesRootDirPath())) {
    QDir dir(Constants::singleton().silkHomePath());
    if (!dir.mkdir("packages")) {
      qCritical() << "failed to create:" << Constants::singleton().userPackagesRootDirPath();
      return false;
    }
  }

  Q_ASSERT(QFileInfo::exists(Constants::singleton().userPackagesRootDirPath()));

  // copy root package.json if it doesn't exist
  if (!QFileInfo::exists(Constants::singleton().userRootPackageJsonPath())) {
    QFile packageJsonInResource(":/root_package.json");
    if (!packageJsonInResource.open(QIODevice::ReadOnly)) {
      qCritical() << "failed to open root_package.json in resource";
      return false;
    }

    QFile rootPackageJson(Constants::singleton().userRootPackageJsonPath());
    if (!rootPackageJson.open(QIODevice::WriteOnly | QIODevice::Text)) {
      qCritical() << "failed to open package.json";
      return false;
    }

    QTextStream in(&packageJsonInResource);
    QTextStream out(&rootPackageJson);
    while (!in.atEnd()) {
      out << in.readLine() << endl;
    }
    out.flush();
  }

  return true;
}

boost::optional<QList<Package>> PackageManager::loadRootPackageJson(const QString& path) {
  QFile rootPackageJson(path);
  if (!rootPackageJson.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return QList<Package>();
  }

  const QJsonDocument& doc = QJsonDocument::fromJson(rootPackageJson.readAll());
  if (doc.isNull()) {
    return boost::none;
  }

  const QJsonObject& rootObj = doc.object();
  QList<Package> packages;
  if (!rootObj.isEmpty() && rootObj.contains(DEPENDENCIES) && rootObj[DEPENDENCIES].isObject()) {
    for (const auto& name : rootObj[DEPENDENCIES].toObject().keys()) {
      QFile pkgJsonPath(QFileInfo(path).dir().absolutePath() + "/node_modules/" + name + "/package.json");
      if (!pkgJsonPath.open(QIODevice::ReadOnly | QIODevice::Text)) {
        continue;
      }
      const QJsonDocument& pkgDoc = QJsonDocument::fromJson(pkgJsonPath.readAll());
      if (pkgDoc.isNull()) {
        continue;
      }

      packages.append(Package::fromJson(pkgDoc.object()));
    }
  }

  return packages;
}

}  // namespace core
