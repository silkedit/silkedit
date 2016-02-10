#include <QDebug>

#include "PackageMenu.h"
#include "PackageParent.h"
#include "PackageAction.h"

namespace core {

PackageMenu::PackageMenu(const QString& title, const QString& pkgName, QWidget* parent)
    : QMenu(title, parent), m_pkgParent(new PackageParent(pkgName, this)) {
  setupConnection();
}

void PackageMenu::setupConnection() {
  connect(this, &QMenu::aboutToShow, [=] {
    for (const auto& action : actions()) {
      if (auto pkgAction = qobject_cast<PackageAction*>(action)) {
        pkgAction->updateVisibilityAndShortcut();
      }
    }
  });
}

PackageMenu::PackageMenu(const QString& title, QWidget* parent)
    : QMenu(title, parent), m_pkgParent(nullptr) {
  setupConnection();
}

}  // namespace core
