#pragma once

#include <string>
#include <functional>
#include <memory>
#include <list>
#include <QMainWindow>

#include "core/macros.h"

class TabView;
class StatusBar;
class ProjectTreeView;
class TabViewGroup;
class FindReplaceView;
class TextEditView;
class Toolbar;
class Console;

namespace core {
class Document;
}

namespace Ui {
class Window;
}
namespace core {
  class Theme;
}

class Window : public QMainWindow {
  Q_OBJECT
  DISABLE_COPY(Window)

 public:
  static Window* create(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  static Window* createWithNewFile(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  static QList<Window*> windows() { return s_windows; }
  static void loadMenu(const QString& pkgName, const QString& ymlPath);

  /**
   * @brief parse toolbars definition and create toolbars for all windows.
   * @param pkgName
   * @param ymlPath
   */
  static void loadToolbar(const QString& pkgName, const QString& ymlPath);

  /**
   * @brief parse toolbars definition and create toolbars for a window.
   * @param window
   * @param pkgName
   * @param ymlPath
   */
  static void loadToolbar(Window* window, const QString& pkgName, const QString& ymlPath);

  static void showFirst();

  static void closeTabIncludingDoc(core::Document* doc);

  ~Window();
  DEFAULT_MOVE(Window)

  // accessor
  TabViewGroup* tabViewGroup() { return m_tabViewGroup; }
  TabView* activeTabView();
  bool isProjectOpend() { return m_projectView != nullptr; }

  void show();
  void closeEvent(QCloseEvent* event) override;
  bool openDir(const QString& dirPath);
  void hideFindReplacePanel();
  QToolBar* findToolbar(const QString& id);

 public slots:
  StatusBar* statusBar();
  Console* console() { return m_console; }
  void openFindAndReplacePanel();

 signals:
  void activeViewChanged(QWidget* oldView, QWidget* newView);
  void firstPaintEventFired();

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  static QList<Window*> s_windows;

  /**
   * @brief toolbars definitions
   * A new window can load toolbars using this toolbars definition map
   */
  static QMap<QString, QString> s_toolbarsDefinitions;

  explicit Window(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);

  std::unique_ptr<Ui::Window> ui;
  TabViewGroup* m_tabViewGroup;
  ProjectTreeView* m_projectView;
  FindReplaceView* m_findReplaceView;
  Console* m_console;
  bool m_firstPaintEventFired;

  static bool closeTabIncludingDocInternal(core::Document* doc);

  void setTheme(const core::Theme* theme);

 private slots:
  void updateConnection(TabView* oldTab, TabView* newTab);
  void updateConnection(QWidget* oldView, QWidget* newView);
  void emitActiveViewChanged(TabView* oldTabView, TabView* newTabView);
};

Q_DECLARE_METATYPE(Window*)
