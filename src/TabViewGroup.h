#pragma once

#include <list>
#include <functional>
#include <QWidget>

#include "macros.h"
#include "UniqueObject.h"

class TabView;
class TabBar;
class Splitter;

class TabViewGroup : public QWidget, public UniqueObject<TabViewGroup> {
  Q_OBJECT
  DISABLE_COPY(TabViewGroup)

 public:
  TabViewGroup(QWidget* parent);
  ~TabViewGroup() = default;
  DEFAULT_MOVE(TabViewGroup)

  // accessor
  TabView* activeTab(bool createIfNull = false);
  void setActiveTab(TabView* tab);

  void saveAllTabs();
  bool closeAllTabs();
  void splitTabHorizontally();
  void splitTabVertically();
  TabBar* tabBarAt(int screenX, int screenY);

signals:
  void activeTabViewChanged(TabView* oldTabView, TabView* newTabView);

 protected:
  friend struct UniqueObject<TabViewGroup>;

  static void request(TabViewGroup* view,
                      const std::string& method,
                      msgpack::rpc::msgid_t msgId,
                      const msgpack::object& obj);
  static void notify(TabViewGroup* view, const std::string& method, const msgpack::object& obj);

 private:
  /**
   * @brief TabView children. This always has at least one TabView
   */
  TabView* m_activeTabView;
  std::list<TabView*> m_tabViews;
  Splitter* m_rootSplitter;

  TabView* createTabView();
  TabView* createInitialTabView();
  void removeTabView(TabView* tab);
  void addTabViewHorizontally(QWidget* widget, const QString& label);
  void addTabViewVertically(QWidget* widget, const QString& label);
  void addTabView(QWidget* widget,
                  const QString& label,
                  Qt::Orientation activeLayoutDirection,
                  Qt::Orientation newDirection);
  void splitTab(std::function<void(QWidget*, const QString&)> func);
};
