#include "PackageMenu.h"
#include "PackageParent.h"

core::PackageMenu::PackageMenu(const QString& title, const QString& pkgName, QWidget* parent)
    : QMenu(title, parent), m_pkgParent(new PackageParent(pkgName, this)) {
}
