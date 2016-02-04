#pragma once

#include <boost/optional.hpp>
#include <QObject>
#include <QString>

#include "macros.h"
#include "Singleton.h"

namespace core {

struct Package;

class PackageManager : public QObject, public Singleton<PackageManager> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(PackageManager)

 public:
  static void loadFiles();
  static boost::optional<QList<Package>> loadPackagesJson(const QByteArray& json);

  ~PackageManager() = default;

 signals:
  void packageRemoved(const Package& pkg);

 private:
  friend class Singleton<PackageManager>;
  PackageManager();
};

}  // namespace core
