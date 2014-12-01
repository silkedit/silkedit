#pragma once

#include <memory>
#include <QMainWindow>
#include <QList>

#include "macros.h"
#include "TextEditView.h"
#include "ViEngine.h"

class STabWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT
  DISABLE_COPY(MainWindow)

 public:
  static MainWindow* create(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  static QList<MainWindow*> windows() { return s_windows; }

  ~MainWindow() = default;
  DEFAULT_MOVE(MainWindow)

  STabWidget* tabBar() { return m_tabbar; }
  void show();

 private:
  static QList<MainWindow*> s_windows;

  MainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  STabWidget* m_tabbar;
};
