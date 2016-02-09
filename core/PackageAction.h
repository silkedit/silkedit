#pragma once

#include <boost/optional.hpp>
#include <QAction>

#include "AndConditionExpression.h"
#include "macros.h"

namespace core {

class PackageParent;

/**
 * @brief QAction with package parent. When parent packgae is removed, this is also removed.
 */
class PackageAction : public QAction {
  Q_OBJECT

 public:
  PackageAction(const QString& text, const QString& pkgName, QObject* parent, boost::optional<AndConditionExpression> cond);
  PackageAction(const QIcon& icon, const QString& text, const QString& pkgName, QObject* parent, boost::optional<AndConditionExpression> cond);
  virtual ~PackageAction() = default;
  DEFAULT_COPY_AND_MOVE(PackageAction)

  void updateVisibility();

 private:
  PackageParent* m_pkgParent;
  boost::optional<AndConditionExpression> m_cond;
};

}  // namespace core
