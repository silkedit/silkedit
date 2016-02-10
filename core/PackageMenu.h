#pragma once

#include <QMenu>
#include "macros.h"

namespace core {

class PackageParent;

/**
 * @brief QMenu with package parent. When parent packgae is removed, this is also removed.
 */
class PackageMenu : public QMenu {
 public:
  PackageMenu(const QString& title, const QString& pkgName, QWidget* parent = nullptr);
  PackageMenu(const QString& title, QWidget* parent = nullptr);
  virtual ~PackageMenu() = default;
  DEFAULT_COPY_AND_MOVE(PackageMenu)

 private:
  PackageParent* m_pkgParent;
  void setupConnection();
};

}  // namespace core
