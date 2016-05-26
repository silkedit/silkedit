#pragma once

#include <list>
#include <functional>
#include <QWidget>
#include <QSettings>
#include <QSplitter>

#include "core/macros.h"

class TabView;
class TabBar;
class Splitter;

class TabViewGroup : public QWidget {
  Q_OBJECT
  Q_PROPERTY(QVector<TabView*> tabViews READ tabViews)
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
  QVector<TabView*> tabViews();
  void saveState(QSettings &settings);
  void loadState(QSettings &settings);

public slots:
  TabView* addNewTabView();
  void splitHorizontally();
  void splitVertically();
  void splitHorizontally(QWidget* initialWidget, const QString& label);
  void splitVertically(QWidget* initialWidget, const QString& label);

 signals:
  void activeTabViewChanged(TabView* oldTabView, TabView* newTabView);
  void currentViewChanged(QWidget* view);

 private:
  TabView* m_activeTabView;
  Splitter* m_rootSplitter;


  TabView* createTabView();
  void removeTabView(TabView* tab);
  void addTabView(QWidget* widget,
                  const QString& label,
                  Qt::Orientation activeLayoutDirection,
                  Qt::Orientation newDirection);
  void splitTextEdit(std::function<void(QWidget*, const QString&)> func);
  void emitCurrentChanged(int index);
  QVector<TabView *> tabViews(QSplitter *splitter);
  void saveState(QSplitter *splitter, QSettings &settings);
  void loadState(QSplitter *splitter, QSettings &settings);
};

Q_DECLARE_METATYPE(TabViewGroup*)
