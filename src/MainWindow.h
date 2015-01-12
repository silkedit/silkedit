#pragma once

#include <functional>
#include <memory>
#include <list>
#include <QMainWindow>

#include "macros.h"
#include "TextEditView.h"
#include "ViEngine.h"

class TabWidget;
class QBoxLayout;
class StatusBar;
class Splitter;
class ProjectTreeView;

class MainWindow : public QMainWindow {
  Q_OBJECT
  DISABLE_COPY(MainWindow)

 public:
  static MainWindow* create(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  static MainWindow* createWithNewFile(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  static QList<MainWindow*> windows() { return s_windows; }

  ~MainWindow();
  DEFAULT_MOVE(MainWindow)

  // accessor
  TabWidget* activeTabWidget() { return m_activeTabWidget; }
  void setActiveTabWidget(TabWidget* tabWidget);

  void show();
  void close();
  void saveAllTabs();
  bool closeAllTabs();
  void splitTabHorizontally();
  void splitTabVertically();
  void closeEvent(QCloseEvent* event) override;
  bool openDir(const QString& dirPath);

 private:
  static QList<MainWindow*> s_windows;

  explicit MainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);

  TabWidget* m_activeTabWidget;
  std::list<TabWidget*> m_tabWidgets;
  Splitter* m_rootSplitter;
  StatusBar* m_statusBar;
  ProjectTreeView* m_projectView;

  TabWidget* createTabWidget();
  void removeTabWidget(TabWidget* widget);
  void addTabWidgetHorizontally(QWidget* widget, const QString& label);
  void addTabWidgetVertically(QWidget* widget, const QString& label);
  void addTabWidget(QWidget* widget,
                    const QString& label,
                    Qt::Orientation activeLayoutDirection,
                    Qt::Orientation newDirection);
  void splitTab(std::function<void(QWidget*, const QString&)> func);
};
