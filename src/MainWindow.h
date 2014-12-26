#pragma once

#include <functional>
#include <memory>
#include <list>
#include <QMainWindow>
#include <QBoxLayout>

#include "macros.h"
#include "TextEditView.h"
#include "ViEngine.h"

class STabWidget;
class QBoxLayout;
class StatusBar;

class MainWindow : public QMainWindow {
  Q_OBJECT
  DISABLE_COPY(MainWindow)

 public:
  static MainWindow* create(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  static QList<MainWindow*> windows() { return s_windows; }

  ~MainWindow();
  DEFAULT_MOVE(MainWindow)

  STabWidget* activeTabWidget() { return m_activeTabWidget; }
  void setActiveTabWidget(STabWidget* tabWidget);
  void show();
  void close();
  void saveAllTabs();
  bool closeAllTabs();
  void splitTabHorizontally();
  void splitTabVertically();
  void closeEvent(QCloseEvent* event) override;

 private:
  static QList<MainWindow*> s_windows;

  explicit MainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  STabWidget* m_activeTabWidget;
  std::list<STabWidget*> m_tabWidgets;
  QBoxLayout* m_rootLayout;
  StatusBar* m_statusBar;

  STabWidget* createTabWidget();
  void removeTabWidget(STabWidget* widget);
  void addTabWidgetHorizontally(QWidget* widget, const QString& label);
  void addTabWidgetVertically(QWidget* widget, const QString& label);
  void addTabWidget(QWidget* widget,
                    const QString& label,
                    QBoxLayout::Direction activeLayoutDirection,
                    QBoxLayout::Direction newDirection);
  void splitTab(std::function<void(QWidget*, const QString&)> func);
};
