#pragma once

#include <string>
#include <functional>
#include <memory>
#include <list>
#include <QMainWindow>

#include "core/macros.h"
#include "core/UniqueObject.h"

class TabView;
class StatusBar;
class ProjectTreeView;
class TabViewGroup;
class FindReplaceView;
class TextEditView;
class Toolbar;
namespace Ui {
class Window;
}

class Window : public QMainWindow, public core::UniqueObject<Window> {
  Q_OBJECT
  DISABLE_COPY(Window)

 public:
  static Window* create(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  static Window* createWithNewFile(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  static QList<Window*> windows() { return s_windows; }
  static void loadMenu(const std::string& pkgName, const std::string& ymlPath);
  static void loadToolbar(const std::string& pkgName, const std::string& ymlPath);

  ~Window();
  DEFAULT_MOVE(Window)

  // accessor
  TabViewGroup* tabViewGroup() { return m_tabViewGroup; }
  TabView* activeTabView();
  StatusBar* statusBar();
  bool isProjectOpend() { return m_projectView != nullptr; }

  void show();
  void closeEvent(QCloseEvent* event) override;
  bool openDir(const QString& dirPath);
  void openFindAndReplacePanel();
  void hideFindReplacePanel();
  QToolBar* findToolbar(const QString& id);

signals:
  void activeEditViewChanged(TextEditView* oldEditView, TextEditView* newEditView);

 protected:
  friend struct UniqueObject<Window>;

  static void request(Window* window,
                      const QString& method,
                      msgpack::rpc::msgid_t msgId,
                      const msgpack::object& obj);
  static void notify(Window* window, const QString& method, const msgpack::object& obj);

 private:
  static QList<Window*> s_windows;

  explicit Window(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);

  std::unique_ptr<Ui::Window> ui;
  TabViewGroup* m_tabViewGroup;
  ProjectTreeView* m_projectView;
  FindReplaceView* m_findReplaceView;

 private slots:
  void updateConnection(TabView* oldTab, TabView* newTab);
  void updateConnection(TextEditView* oldEditView, TextEditView* newEditView);
  void emitActiveEditViewChanged(TabView* oldTabView, TabView* newTabView);
};
