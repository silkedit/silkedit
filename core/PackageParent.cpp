#include "PackageParent.h"
#include "PackageManager.h"
#include "Package.h"

core::PackageParent::PackageParent(const QString& pkgName, QObject* child)
    : QObject(child), m_pkgName(pkgName), m_child(child) {
  connect(&PackageManager::singleton(), &PackageManager::packageRemoved, this,
          &PackageParent::remove);
}

void core::PackageParent::remove(const Package& pkg) {
  if (pkg.name == m_pkgName && m_child) {
    m_child->deleteLater();
  }
}
