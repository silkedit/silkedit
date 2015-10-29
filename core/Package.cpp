#include <QJsonObject>
#include <QStringList>
#include <QRegularExpression>

#include "Package.h"

core::Package::Package(const QJsonValue& jsonValue) {
  QJsonObject jsonObj = jsonValue.toObject();
  name = jsonObj["name"].toString();
  version = jsonObj["version"].toString();
  description = jsonObj["description"].toString();
  QJsonValue repoValue = jsonObj["repository"];
  if (repoValue.isObject()) {
    QJsonObject repoObj = repoValue.toObject();
    // e.g., git+https://github.com/silkedit/hello.git
    githubUrl = "git+" + repoObj["url"].toString();
  } else {
    // e.g., silkedit/hello
    githubUrl = repoValue.toString();
  }
}

QStringList core::Package::validate() {
  QStringList errors;
  if (name.isEmpty()) {
    errors << "package name is empty";
  }

  if (githubUrl.isEmpty()) {
    errors << "packgae repository is empty";
  }

  if (version.isEmpty()) {
    errors << "package version is empty";
  }

  return errors;
}
