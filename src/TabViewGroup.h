#pragma once

#include <list>
#include <functional>
#include <QWidget>
#include <QLinkedList>

#include "core/macros.h"

class TabView;
class TabBar;
class Splitter;

class TabViewGroup : public QWidget {
  Q_OBJECT
  Q_PROPERTY(QLinkedList<TabView*> tabViews READ tabViews)
  DISABLE_COPY(TabViewGroup)

 public:
  TabViewGroup(QWidget* parent);
  ~TabViewGroup() = default;
  DEFAULT_MOVE(TabViewGroup)

  // accessor
  TabView* activeTab();
  void setActiveTab(TabView* newTabView);

  bool closeAllTabs();
  TabBar* tabBarAt(int screenX, int screenY);
  QLinkedList<TabView*> tabViews() { return m_tabViews; }


public slots:
  void splitHorizontally();
  void splitVertically();
  void splitHorizontally(QWidget* initialWidget, const QString& label);
  void splitVertically(QWidget* initialWidget, const QString& label);

 signals:
  void activeTabViewChanged(TabView* oldTabView, TabView* newTabView);
  void currentViewChanged(QWidget* view);

 private:
  TabView* m_activeTabView;
  /**
   * @brief TabView children. This always has at least one TabView
   */
  QLinkedList<TabView*> m_tabViews;
  Splitter* m_rootSplitter;

  TabView* createTabView();
  TabView* createInitialTabView();
  void removeTabView(TabView* tab);
  void addTabView(QWidget* widget,
                  const QString& label,
                  Qt::Orientation activeLayoutDirection,
                  Qt::Orientation newDirection);
  void splitTextEditView(std::function<void(QWidget*, const QString&)> func);
  void emitCurrentChanged(int index);
};

Q_DECLARE_METATYPE(TabViewGroup*)
