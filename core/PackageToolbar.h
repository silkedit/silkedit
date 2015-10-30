#pragma once

#include <QToolBar>

#include "macros.h"

namespace core {

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
  PackageParent* m_pkgParent;
};

}  // namespace core
