#include "PackageAction.h"
#include "PackageParent.h"

core::PackageAction::PackageAction(const QString& text,
                                   const QString& pkgName,
                                   QObject* parent,
                                   boost::optional<AndConditionExpression> cond)
    : QAction(text, parent), m_pkgParent(new PackageParent(pkgName, this)), m_cond(cond) {
  updateVisibility();
}

core::PackageAction::PackageAction(const QIcon& icon,
                                   const QString& text,
                                   const QString& pkgName,
                                   QObject* parent,
                                   boost::optional<AndConditionExpression> cond)
    : QAction(icon, text, parent), m_pkgParent(new PackageParent(pkgName, this)), m_cond(cond) {
  updateVisibility();
}

void core::PackageAction::updateVisibility()
{
  if (m_cond) {
    setVisible((*m_cond).isSatisfied());
  }
}
