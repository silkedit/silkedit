#pragma once

#include <array>
#include <list>
#include <memory>
#include <QAction>
#include <QObject>
#include <QMenu>

#include "OpenRecentItemManager.h"
#include "macros.h"
#include "Singleton.h"

class ClearRecentItemListAction;

class OpenRecentItemManager : public QObject, public Singleton<OpenRecentItemManager> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(OpenRecentItemManager)

 public:
  ~OpenRecentItemManager() = default;

  QMenu* openRecentMenu() { return m_openRecentMenu.get(); }
  void clear();
  void reopenLastClosedFile();

 public slots:
  void addOpenRecentItem(const QString& path);

 private:
  static const int MAX_RECENT_ITEMS = 5;

  friend class Singleton<OpenRecentItemManager>;
  OpenRecentItemManager();

  std::array<QAction*, MAX_RECENT_ITEMS> m_recentItemActions;
  std::list<QString> m_recentItems;
  std::unique_ptr<QMenu> m_openRecentMenu;
  ClearRecentItemListAction* m_clearRecentItemListAction;
  QAction* m_reopenLastClosedFileAction;

  void updateOpenRecentItems();
};

class OpenRecentAction : public QAction {
  DISABLE_COPY(OpenRecentAction)
 public:
  explicit OpenRecentAction(QObject* parent = nullptr);
  ~OpenRecentAction() = default;
  DEFAULT_MOVE(OpenRecentAction)
};

class ClearRecentItemListAction : public QAction {
  DISABLE_COPY(ClearRecentItemListAction)
 public:
  explicit ClearRecentItemListAction(QObject* parent = nullptr);
  ~ClearRecentItemListAction() = default;
  DEFAULT_MOVE(ClearRecentItemListAction)
};
