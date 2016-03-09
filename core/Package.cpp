#include <boost/optional.hpp>
#include <QJsonObject>
#include <QStringList>
#include <QRegularExpression>

#include "Package.h"

namespace {
const QString descriptionKey = QStringLiteral("description");

boost::optional<QJsonObject> findLocaleObj(const QJsonObject& jsonObj, const QString& locale) {
  if (!locale.isEmpty()) {
    auto objValue = jsonObj[QStringLiteral("locales")];
    if (objValue.isObject()) {
      auto obj = objValue.toObject();
      auto localeObjValue = obj[locale];
      if (localeObjValue.isObject()) {
        return localeObjValue.toObject();
      } else {
        int index = locale.indexOf('_');
        if (index >= 0) {
          auto localeObjValue = obj[locale.left(index)];
          if (localeObjValue.isObject()) {
            return localeObjValue.toObject();
          }
        }
      }
    }
  }

  return boost::none;
}
}

core::Package::Package(const QJsonValue& jsonValue, const QString& locale) {
  QJsonObject jsonObj = jsonValue.toObject();
  auto maybeLocaleObj = findLocaleObj(jsonObj, locale);
  name = jsonObj[QStringLiteral("name")].toString();
  version = jsonObj[QStringLiteral("version")].toString();
  description = maybeLocaleObj && (*maybeLocaleObj).contains(descriptionKey)
                    ? (*maybeLocaleObj)[descriptionKey].toString()
                    : jsonObj[QStringLiteral("description")].toString();
  QJsonValue repoValue = jsonObj[QStringLiteral("repository")];
  if (repoValue.isObject()) {
    QJsonObject repoObj = repoValue.toObject();
    // e.g., https://github.com/silkedit/hello.git
    repositoryUrl = repoObj[QStringLiteral("url")].toString();
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
