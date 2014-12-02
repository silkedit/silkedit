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

  ~MainWindow();
  DEFAULT_MOVE(MainWindow)

  STabWidget* tabWidget() { return m_tabWidget; }
  void show();
  void close();

 private:
  static QList<MainWindow*> s_windows;

  MainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  STabWidget* m_tabWidget;
};
