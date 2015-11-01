#pragma once

#include <QAction>
#include "macros.h"

namespace core {

class PackageParent;

/**
 * @brief QAction with package parent. When parent packgae is removed, this is also removed.
 */
class PackageAction : public QAction {
  Q_OBJECT

 public:
  PackageAction(const QString& text, const QString& pkgName, QObject* parent);
  PackageAction(const QIcon& icon, const QString& text, const QString& pkgName, QObject* parent);
  virtual ~PackageAction() = default;
  DEFAULT_COPY_AND_MOVE(PackageAction)

 private:
  PackageParent* m_pkgParent;
};

}  // namespace core
