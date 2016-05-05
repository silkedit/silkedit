#pragma once

#include <QToolBar>

#include "macros.h"

namespace core {
class Theme;
class PackageParent;

class PackageToolBar : public QToolBar {
  DISABLE_COPY(PackageToolBar)

 public:
  PackageToolBar(const QString& objectName,
                 const QString& title,
                 QWidget* parent,
                 const QString& pkgName);
  ~PackageToolBar() = default;
  DEFAULT_MOVE(PackageToolBar)

 private:
  void setTheme(const Theme* theme);
  PackageParent* m_pkgParent;
};

}  // namespace core
