#include <QJsonObject>
#include <QStringList>
#include <QRegularExpression>

#include "Package.h"

core::Package::Package(const QJsonValue& jsonValue) {
  QJsonObject jsonObj = jsonValue.toObject();
  name = jsonObj["name"].toString();
  version = jsonObj["version"].toString();
  description = jsonObj["description"].toString();
  repository = jsonObj["repository"].toString();
}

QStringList core::Package::validate() {
  QStringList errors;
  if (name.isEmpty()) {
    errors << "package name is empty";
  }

  if (repository.isEmpty()) {
    errors << "packgae repository is empty";
  }

  if (version.isEmpty()) {
    errors << "package version is empty";
  }

  return errors;
}

QString core::Package::zipUrl() {
  QString owner;
  QString repo;
  QRegularExpression regex(R"(^https://github.com/([^/]+)/([^/]+)$)");
  QRegularExpressionMatch match = regex.match(repository);
  if (!match.hasMatch()) {
    qWarning("no match of owner and repo. reposiotry: %s", qPrintable(repository));
    return "";
  }
  owner = match.captured(1);
  repo = match.captured(2);
  return QString("https://api.github.com/repos/%1/%2/zipball/%3").arg(owner, repo, version);
}
