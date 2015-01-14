#pragma once

#include <functional>
#include <memory>
#include <list>
#include <QMainWindow>

#include "macros.h"

class TabView;
class QBoxLayout;
class StatusBar;
class Splitter;
class ProjectTreeView;
class TabViewGroup;
class FindReplaceView;
class TextEditView;

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
  TabViewGroup* activeTabViewGroup() { return m_tabViewGroup; }
  TabView* activeTabView();

  void show();
  void close();
  void closeEvent(QCloseEvent* event) override;
  bool openDir(const QString& dirPath);

signals:
  void activeEditViewChanged(TextEditView* oldEditView, TextEditView* newEditView);

 private:
  static QList<MainWindow*> s_windows;

  explicit MainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);

  Splitter* m_rootSplitter;
  TabViewGroup* m_tabViewGroup;
  StatusBar* m_statusBar;
  ProjectTreeView* m_projectView;
  FindReplaceView* m_findReplaceView;

 private slots:
  void updateConnection(TabView* oldTab, TabView* newTab);
  void updateConnection(TextEditView* oldEditView, TextEditView* newEditView);
  void emitActiveEditViewChanged(TabView* oldTabView, TabView* newTabView);
};
