#pragma once

#include <string>
#include <functional>
#include <memory>
#include <list>
#include <QMainWindow>

#include "macros.h"
#include "UniqueObject.h"

class TabView;
class QBoxLayout;
class StatusBar;
class Splitter;
class ProjectTreeView;
class TabViewGroup;
class FindReplaceView;
class TextEditView;

class MainWindow : public QMainWindow, public UniqueObject<MainWindow> {
  Q_OBJECT
  DISABLE_COPY(MainWindow)

 public:
  static MainWindow* create(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  static MainWindow* createWithNewFile(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  static QList<MainWindow*> windows() { return s_windows; }
  static void loadMenu(const std::string& ymlPath);

  ~MainWindow();
  DEFAULT_MOVE(MainWindow)

  // accessor
  TabViewGroup* tabViewGroup() { return m_tabViewGroup; }
  TabView* activeTabView();

  void show();
  void closeEvent(QCloseEvent* event) override;
  bool openDir(const QString& dirPath);
  void openFindAndReplacePanel();
  void hideFindReplacePanel();

signals:
  void activeEditViewChanged(TextEditView* oldEditView, TextEditView* newEditView);

 protected:
  friend struct UniqueObject<MainWindow>;

  static void request(const std::string& method, msgpack::rpc::msgid_t msgId, MainWindow* window);
  static void notify(const std::string& method, MainWindow* window);

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
