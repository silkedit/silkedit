#pragma once

#include <list>
#include <functional>
#include <QWidget>

#include "core/macros.h"
#include "core/UniqueObject.h"

class TabView;
class TabBar;
class Splitter;

class TabViewGroup : public QWidget, public core::UniqueObject {
  Q_OBJECT
  DISABLE_COPY(TabViewGroup)

 public:
  TabViewGroup(QWidget* parent);
  ~TabViewGroup() = default;
  DEFAULT_MOVE(TabViewGroup)

  // accessor
  TabView* activeTab();
  void setActiveTab(TabView* tab);

  Q_INVOKABLE void saveAll();
  bool closeAllTabs();
  Q_INVOKABLE void splitTabHorizontally();
  Q_INVOKABLE void splitTabVertically();
  TabBar* tabBarAt(int screenX, int screenY);

 signals:
  void activeTabViewChanged(TabView* oldTabView, TabView* newTabView);

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

Q_DECLARE_METATYPE(TabViewGroup*)
