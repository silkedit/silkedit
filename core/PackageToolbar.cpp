#include "PackageToolBar.h"
#include "PackageParent.h"

core::PackageToolBar::PackageToolBar(const QString& objectName,
                                     const QString& title,
                                     QWidget* parent,
                                     const QString& pkgName)
    : QToolBar(title, parent), m_pkgParent(new PackageParent(pkgName, this)) {
  setObjectName(objectName);
}
