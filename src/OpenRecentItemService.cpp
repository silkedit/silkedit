#include <qDebug>

#include "OpenRecentItemService.h"
#include "DocumentService.h"

void OpenRecentItemService::clear() {
  m_recentItems.clear();
  updateOpenRecentItems();
}

void OpenRecentItemService::addOpenRecentItem(const QString& path) {
  if (path.isEmpty()) {
    qDebug("path is empty");
    return;
  }

  if (std::find(m_recentItems.begin(), m_recentItems.end(), path) != m_recentItems.end()) {
    qDebug() << path << "is already in recent file list";
    return;
  }

  m_recentItems.push_front(path);
  updateOpenRecentItems();
}

OpenRecentItemService::OpenRecentItemService() : m_openRecentMenu(new QMenu(tr("Open Recent"))) {
  for (int i = 0; i < MAX_RECENT_ITEMS; i++) {
    auto action = new OpenRecentAction(m_openRecentMenu.get());
    action->setVisible(false);
    m_recentItemActions[i] = action;
    m_openRecentMenu->addAction(action);
  }

  m_openRecentMenu->addSeparator();
  m_clearRecentItemListAction = new ClearRecentItemListAction(m_openRecentMenu.get());
  m_openRecentMenu->addAction(m_clearRecentItemListAction);

  updateOpenRecentItems();
}

void OpenRecentItemService::updateOpenRecentItems() {
  // delete extra recent items
  while (m_recentItems.size() > MAX_RECENT_ITEMS) {
    m_recentItems.pop_back();
  }

  m_clearRecentItemListAction->setEnabled(m_recentItems.empty() ? false : true);

  int index = 0;
  for (auto& item : m_recentItems) {
    qDebug("add recent menu item");
    m_recentItemActions[index]->setText(item);
    m_recentItemActions[index]->setData(item);
    m_recentItemActions[index]->setVisible(true);
    index++;
  }

  // Hide empty recent item actions
  for (int i = index; i < MAX_RECENT_ITEMS; i++) {
    m_recentItemActions[i]->setVisible(false);
  }
}

OpenRecentAction::OpenRecentAction(QObject* parent) : QAction(parent) {
  QObject::connect(this, &QAction::triggered, [this]() {
    if (data().isValid()) {
      DocumentService::singleton().open(data().toString());
    }
  });
}

ClearRecentItemListAction::ClearRecentItemListAction(QObject* parent)
    : QAction(tr("Clear List"), parent) {
  QObject::connect(
      this, &QAction::triggered, [this]() { OpenRecentItemService::singleton().clear(); });
}
