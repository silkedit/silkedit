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
    // e.g., https://github.com/silkedit/hello.git
    repositoryUrl = repoObj["url"].toString();
  } else {
    // e.g., silkedit/hello (Github shortcut url)
    repositoryUrl = repoValue.toString();
  }
}

QStringList core::Package::validate() {
  QStringList errors;
  if (name.isEmpty()) {
    errors << "package name is empty";
  }

  if (repositoryUrl.isEmpty()) {
    errors << "packgae repository is empty";
  }

  if (version.isEmpty()) {
    errors << "package version is empty";
  }

  return errors;
}

QString core::Package::tarballUrl() const {
  QString owner;
  QString repo;
  // +? is non-greedy match
  QRegularExpression regex(R"(^(https://github.com/)?(?<owner>[^/]+)/(?<repo>[^/]+?)(.git)?$)");
  QRegularExpressionMatch match = regex.match(repositoryUrl);
  if (!match.hasMatch()) {
    qWarning("no match of owner and repo. reposiotry: %s", qPrintable(repositoryUrl));
    return "";
  }
  owner = match.captured("owner");
  repo = match.captured("repo");
  return QString("https://github.com/%1/%2/tarball/%3").arg(owner, repo, version);
}

QJsonObject core::Package::toJson() const {
  QJsonObject obj;
  obj["name"] = name;
  obj["version"] = version;
  obj["description"] = description;
  obj["repository"] = repositoryUrl;
  return obj;
}
