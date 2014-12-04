#include "OpenRecentItemService.h"

namespace {
const int MAX_RECENT_ITEMS = 5;
}

void OpenRecentItemService::updateOpenRecentItem(const QString& path) {
  qDebug("updateOpenRecentItem");
  if (path.isEmpty()) {
    qDebug("path is empty");
    return;
  }

  m_openRecentMenu->clear();

  auto action = std::unique_ptr<QAction>(new QAction(nullptr));
  action->setText(path);
  m_recentItemActions.push_front(std::move(action));

  while (m_recentItemActions.size() > MAX_RECENT_ITEMS) {
    m_recentItemActions.pop_back();
  }

  for (auto& action : m_recentItemActions) {
    qDebug("add recent menu item");
    m_openRecentMenu->addAction(action.get());
  }
}

OpenRecentItemService::OpenRecentItemService() : m_openRecentMenu(new QMenu(tr("Open Recent"))) {
}
