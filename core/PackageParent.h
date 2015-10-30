#pragma once

#include <QObject>
#include "macros.h"

namespace core {

struct Package;

/**
 * @brief When the packgae is removed, this deletes its child object.
 */
class PackageParent : public QObject {
  Q_OBJECT
 public:
  PackageParent(const QString& pkgName, QObject* child);
  virtual ~PackageParent() = default;
  DEFAULT_COPY_AND_MOVE(PackageParent)

 private:
  QString m_pkgName;
  QObject* m_child;

  void remove(const Package& pkg);
};

}  // namespace core
