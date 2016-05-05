#include "PackageToolBar.h"
#include "PackageParent.h"
#include "Config.h"
#include "Theme.h"

namespace core {

PackageToolBar::PackageToolBar(const QString& objectName,
                               const QString& title,
                               QWidget* parent,
                               const QString& pkgName)
    : QToolBar(title, parent), m_pkgParent(new PackageParent(pkgName, this)) {
  setObjectName(objectName);
  setTheme(Config::singleton().theme());
  connect(&Config::singleton(), &Config::themeChanged, this, &PackageToolBar::setTheme);
}

void PackageToolBar::setTheme(const Theme* theme) {
  qDebug("PackageToolBar theme is changed");
  if (!theme) {
    qWarning("theme is null");
    return;
  }

  if (theme->packageToolBarSettings != nullptr) {
    QString style;
    ColorSettings* packageToolBarSettings = theme->packageToolBarSettings.get();

    style = QString(
                "QToolBar {"
                "background-color: %1;"
                "color: %2;"
                "}")
                .arg(packageToolBarSettings->value("background").name(),
                     packageToolBarSettings->value("foreground").name());

    this->setStyleSheet(style);
  }
}

}  // namespace core
