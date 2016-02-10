#include "PackageAction.h"
#include "PackageParent.h"

core::PackageAction::PackageAction(const QString& text,
                                   const QString& pkgName,
                                   QObject* parent,
                                   boost::optional<AndConditionExpression> cond)
    : QAction(text, parent), m_pkgParent(new PackageParent(pkgName, this)), m_cond(cond) {
  updateVisibilityAndShortcut();
}

core::PackageAction::PackageAction(const QIcon& icon,
                                   const QString& text,
                                   const QString& pkgName,
                                   QObject* parent,
                                   boost::optional<AndConditionExpression> cond)
    : QAction(icon, text, parent), m_pkgParent(new PackageParent(pkgName, this)), m_cond(cond) {
  updateVisibilityAndShortcut();
}

void core::PackageAction::updateVisibilityAndShortcut()
{
  if (m_cond) {
    setVisible((*m_cond).isSatisfied());
  }
}
