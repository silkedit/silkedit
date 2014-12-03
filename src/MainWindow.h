#pragma once

#include <memory>
#include <QMainWindow>
#include <QList>

#include "macros.h"
#include "TextEditView.h"
#include "ViEngine.h"

class STabWidget;
class QBoxLayout;

class MainWindow : public QMainWindow {
  Q_OBJECT
  DISABLE_COPY(MainWindow)

 public:
  static MainWindow* create(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  static QList<MainWindow*> windows() { return s_windows; }

  ~MainWindow();
  DEFAULT_MOVE(MainWindow)

  STabWidget* activeTabWidget() { return m_activeTabWidget; }
  void show();
  void close();
  void splitTabHorizontally();

 private:
  static QList<MainWindow*> s_windows;

  MainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  STabWidget* m_activeTabWidget;
  QList<STabWidget*> m_tabWidgets;
  QBoxLayout* m_layout;

  STabWidget* createTabWidget();
  void removeTabWidget(STabWidget* widget);
  void addTabWidgetHorizontally(QWidget* widget, const QString& label);
  //  void addTabWidgetVertically();
};
