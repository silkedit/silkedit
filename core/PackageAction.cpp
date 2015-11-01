#include "PackageAction.h"
#include "PackageParent.h"

core::PackageAction::PackageAction(const QString& text, const QString& pkgName, QObject* parent)
    : QAction(text, parent), m_pkgParent(new PackageParent(pkgName, this)) {
}

core::PackageAction::PackageAction(const QIcon& icon,
                                   const QString& text,
                                   const QString& pkgName,
                                   QObject* parent)
    : QAction(icon, text, parent), m_pkgParent(new PackageParent(pkgName, this)) {
}
