#pragma once

#include <list>
#include <memory>
#include <QAction>
#include <QObject>
#include <QMenu>

#include "macros.h"
#include "Singleton.h"

class OpenRecentItemService : public QObject, public Singleton<OpenRecentItemService> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(OpenRecentItemService)

 public:
  ~OpenRecentItemService() = default;

  QMenu* openRecentMenu() { return m_openRecentMenu.get(); }

 public slots:
  void updateOpenRecentItem(const QString& path);

 private:
  friend class Singleton<OpenRecentItemService>;
  OpenRecentItemService();

  std::list<std::unique_ptr<QAction>> m_recentItemActions;
  std::unique_ptr<QMenu> m_openRecentMenu;
};

class OpenRecentAction : public QAction {
  DISABLE_COPY(OpenRecentAction)
  public:
    explicit OpenRecentAction(QObject* parent = nullptr);
    ~OpenRecentAction() = default;
    DEFAULT_MOVE(OpenRecentAction)
};
