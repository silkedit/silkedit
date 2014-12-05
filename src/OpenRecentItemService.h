#pragma once

#include <array>
#include <list>
#include <memory>
#include <QAction>
#include <QObject>
#include <QMenu>

#include "OpenRecentItemService.h"
#include "macros.h"
#include "Singleton.h"

class ClearRecentItemListAction;

class OpenRecentItemService : public QObject, public Singleton<OpenRecentItemService> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(OpenRecentItemService)

 public:
  ~OpenRecentItemService() = default;

  QMenu* openRecentMenu() { return m_openRecentMenu.get(); }
  void clear();

 public slots:
  void addOpenRecentItem(const QString& path);

 private:
  static const int MAX_RECENT_ITEMS = 5;

  friend class Singleton<OpenRecentItemService>;
  OpenRecentItemService();

  std::array<QAction*, MAX_RECENT_ITEMS> m_recentItemActions;
  std::list<QString> m_recentItems;
  std::unique_ptr<QMenu> m_openRecentMenu;
  ClearRecentItemListAction* m_clearRecentItemListAction;

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
