#pragma once

#include <list>
#include <QWidget>

#include "macros.h"

class TabView;
class Splitter;

class TabViewGroup : public QWidget {
  Q_OBJECT
  DISABLE_COPY(TabViewGroup)

 public:
  TabViewGroup(QWidget* parent);
  ~TabViewGroup() = default;
  DEFAULT_MOVE(TabViewGroup)

  // accessor
  TabView* activeTab() { return m_activeTabView; }
  void setActiveTab(TabView* tab);

  void saveAllTabs();
  bool closeAllTabs();
  void splitTabHorizontally();
  void splitTabVertically();

signals:
  void activeTabViewChanged(TabView* oldTabView, TabView* newTabView);

 private:
  TabView* m_activeTabView;
  std::list<TabView*> m_tabViews;
  Splitter* m_rootSplitter;

  TabView* createTabView();
  void removeTabView(TabView* tab);
  void addTabViewHorizontally(QWidget* widget, const QString& label);
  void addTabViewVertically(QWidget* widget, const QString& label);
  void addTabView(QWidget* widget,
                  const QString& label,
                  Qt::Orientation activeLayoutDirection,
                  Qt::Orientation newDirection);
  void splitTab(std::function<void(QWidget*, const QString&)> func);
};
