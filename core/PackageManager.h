#pragma once

#include <boost/optional.hpp>
#include <QObject>
#include <QString>
#include <QMap>

#include "macros.h"
#include "Singleton.h"

namespace core {

struct Package;

class PackageManager : public QObject, public Singleton<PackageManager> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(PackageManager)

 public:
  static const QString DEPENDENCIES;

  // for installed packages
  static boost::optional<QList<Package>> loadRootPackageJson(const QString& path);

  // for available packages
  static boost::optional<QList<Package>> loadPackagesJson(const QByteArray& json);

  ~PackageManager() = default;

  void loadFiles();

  QMap<QString, QString> toolbarDefinitions() { return m_toolbarsDefinitions; }

public slots:
  // This is called from JS internally
  bool _ensureRootPackageJson();

 signals:
  void packageRemoved(const Package& pkg);

 private:
  friend class Singleton<PackageManager>;
  PackageManager();

  QMap<QString, QString> m_toolbarsDefinitions;
  void loadPackageContents(const QString &pkgPath, const QString &pkg);
};

}  // namespace core
