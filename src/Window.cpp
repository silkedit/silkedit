﻿#include <yaml-cpp/yaml.h>
#include <QFileDialog>
#include <QApplication>
#include <QDebug>
#include <QVBoxLayout>
#include <QToolBar>

#include "Window.h"
#include "ui_Window.h"
#include "TabViewGroup.h"
#include "TextEditView.h"
#include "StatusBar.h"
#include "ProjectTreeView.h"
#include "Splitter.h"
#include "TabView.h"
#include "FindReplaceView.h"
#include "MenuBar.h"
#include "CommandAction.h"
#include "util/YamlUtil.h"
#include "Helper.h"
#include "PlatformUtil.h"
#include "Console.h"
#include "core/Document.h"

QMap<QString, QString> Window::s_toolbarsDefinitions;

Window::Window(QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      ui(new Ui::Window),
      m_tabViewGroup(new TabViewGroup(this)),
      m_projectView(nullptr),
      m_findReplaceView(new FindReplaceView(this)),
      m_console(new Console(this)),
      m_firstPaintEventFired(false) {
  qDebug("creating Window");
  ui->setupUi(this);

  // QWebEngineView doesn't work well with unified toolbar on Mac
  // https://bugreports.qt.io/browse/QTBUG-41179
  // setUnifiedTitleAndToolBarOnMac(true);
  setAttribute(Qt::WA_DeleteOnClose);

// Note: Windows of Mac app share global menu bar
#ifndef Q_OS_MAC
  // Copy menus from global menu bar
  QList<QAction*> actions = MenuBar::globalMenuBar()->actions();
  menuBar()->insertActions(nullptr, actions);
#endif

  // Setup toolbars for this window from toolbars definitions
  for (const auto& pkgName : s_toolbarsDefinitions.keys()) {
    loadToolbar(this, pkgName, s_toolbarsDefinitions.value(pkgName));
  }

  ui->rootSplitter->setContentsMargins(0, 0, 0, 0);

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->setMargin(0);
  m_tabViewGroup->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  layout->addWidget(m_tabViewGroup);
  layout->addWidget(m_findReplaceView);
  m_findReplaceView->hide();
  QWidget* editorWidget = new QWidget(this);
  editorWidget->setLayout(layout);

  auto contentSplitter = new QSplitter(Qt::Vertical);
  contentSplitter->addWidget(editorWidget);
  contentSplitter->addWidget(m_console);
  m_console->hide();
  contentSplitter->setSizes(QList<int>{500, 100});

  ui->rootSplitter->addWidget(contentSplitter);

  connect(m_tabViewGroup, &TabViewGroup::activeTabViewChanged, this,
          static_cast<void (Window::*)(TabView*, TabView*)>(&Window::updateConnection));
  connect(m_tabViewGroup, &TabViewGroup::activeTabViewChanged, this,
          &Window::emitActiveViewChanged);
  connect(m_tabViewGroup, &TabViewGroup::currentViewChanged, m_findReplaceView,
          &FindReplaceView::setActiveView);
  connect(this, &Window::activeViewChanged, ui->statusBar, &StatusBar::onActiveViewChanged);

  updateConnection(nullptr, m_tabViewGroup->activeTab());
}

void Window::loadToolbar(const QString& pkgName, const QString& ymlPath) {
  qDebug("Start loading. pkg: %s, path: %s", qPrintable(pkgName), qPrintable(ymlPath));
  foreach (Window* win, s_windows) { loadToolbar(win, pkgName, ymlPath); }
}

void Window::updateConnection(TabView* oldTabView, TabView* newTabView) {
  //  qDebug("updateConnection for new active TabView");

  if (oldTabView && ui->statusBar) {
    disconnect(oldTabView, &TabView::activeViewChanged, ui->statusBar,
               &StatusBar::onActiveViewChanged);
    disconnect(oldTabView, &TabView::activeViewChanged, this,
               static_cast<void (Window::*)(QWidget*, QWidget*)>(&Window::updateConnection));
  }

  if (newTabView && ui->statusBar) {
    connect(newTabView, &TabView::activeViewChanged, ui->statusBar,
            &StatusBar::onActiveViewChanged);
    connect(newTabView, &TabView::activeViewChanged, this,
            static_cast<void (Window::*)(QWidget*, QWidget*)>(&Window::updateConnection));
  }
}

void Window::updateConnection(QWidget* oldView, QWidget* newView) {
  //  qDebug("updateConnection for new active TextEditView");

  TextEditView* oldEditView = qobject_cast<TextEditView*>(oldView);
  if (oldEditView && ui->statusBar) {
    disconnect(oldEditView, &TextEditView::languageChanged, ui->statusBar, &StatusBar::setLanguage);
    disconnect(oldEditView, &TextEditView::encodingChanged, ui->statusBar, &StatusBar::setEncoding);
    disconnect(oldEditView, &TextEditView::lineSeparatorChanged, ui->statusBar,
               &StatusBar::setLineSeparator);
    disconnect(oldEditView, &TextEditView::bomChanged, ui->statusBar, &StatusBar::setBOM);
  }

  TextEditView* newEditView = qobject_cast<TextEditView*>(newView);
  if (newEditView && ui->statusBar) {
    connect(newEditView, &TextEditView::languageChanged, ui->statusBar, &StatusBar::setLanguage);
    connect(newEditView, &TextEditView::encodingChanged, ui->statusBar, &StatusBar::setEncoding);
    connect(newEditView, &TextEditView::lineSeparatorChanged, ui->statusBar,
            &StatusBar::setLineSeparator);
    connect(newEditView, &TextEditView::bomChanged, ui->statusBar, &StatusBar::setBOM);
  }
}

void Window::emitActiveViewChanged(TabView* oldTabView, TabView* newTabView) {
  QWidget* oldView = oldTabView ? oldTabView->activeView() : nullptr;
  QWidget* newView = newTabView ? newTabView->activeView() : nullptr;
  emit activeViewChanged(oldView, newView);
}

Window* Window::create(QWidget* parent, Qt::WindowFlags flags) {
  Window* window = new Window(parent, flags);
  s_windows.append(window);
  return window;
}

Window* Window::createWithNewFile(QWidget* parent, Qt::WindowFlags flags) {
  Window* w = create(parent, flags);
  bool result = false;

  result = w->activeTabView()->createWithSavedTabs();

  if (!result) {
    w->activeTabView()->addNew();
  }

  return w;
}

void Window::loadMenu(const QString& pkgName, const QString& ymlPath) {
  qDebug("Start loading. pkg: %s, path: %s", qPrintable(pkgName), qPrintable(ymlPath));
  try {
    YAML::Node rootNode = YAML::LoadFile(ymlPath.toUtf8().constData());
    if (!rootNode.IsMap()) {
      qWarning("root node must be a map");
      return;
    }

    YAML::Node menuNode = rootNode["menu"];
#ifdef Q_OS_MAC
    // There's only 1 global menu bar on Mac.
    YamlUtil::parseMenuNode(pkgName, MenuBar::globalMenuBar(), menuNode);
#elif defined Q_OS_WIN
    // Menu bar belongs to each window.
    foreach (Window* win, s_windows) { YamlUtil::parseMenuNode(pkgName, win->menuBar(), menuNode); }
#endif
  } catch (const YAML::ParserException& ex) {
    qWarning("Unable to load %s. Cause: %s", qPrintable(ymlPath), ex.what());
  }
}

// todo: remove yaml-cpp dependency in Window class
void Window::loadToolbar(Window* win, const QString& pkgName, const QString& ymlPath) {
  try {
    YAML::Node rootNode = YAML::LoadFile(ymlPath.toUtf8().constData());
    if (!rootNode.IsMap()) {
      qWarning("root node must be a map");
      return;
    }

    s_toolbarsDefinitions.insert(pkgName, ymlPath);

    YAML::Node toolbarsNode = rootNode["toolbar"];
    YamlUtil::parseToolbarNode(pkgName, ymlPath, win, toolbarsNode);
  } catch (const YAML::ParserException& ex) {
    qWarning("Unable to load %s. Cause: %s", qPrintable(ymlPath), ex.what());
  }
}

void Window::showFirst() {
  if (s_windows.size() > 0) {
    Window* win = s_windows.first();
    win->show();
    win->raise();
    win->activateWindow();
  }
}

bool Window::closeTabIncludingDocInternal(core::Document* doc) {
  for (Window* win : Window::windows()) {
    Q_ASSERT(win);
    for (TabView* tab : win->tabViewGroup()->tabViews()) {
      Q_ASSERT(tab);
      TabView::CloseTabIncludingDocResult result = tab->closeTabIncludingDoc(doc);
      switch (result) {
        case TabView::CloseTabIncludingDocResult::AllTabsRemoved:
          return true;
        case TabView::CloseTabIncludingDocResult::UserCanceled:
          return false;
        default:
          break;
      }
    }
  }

  return false;
}

void Window::closeTabIncludingDoc(core::Document* doc) {
  bool needsRetry = false;
  do {
    needsRetry = closeTabIncludingDocInternal(doc);
  } while (needsRetry);
}

Window::~Window() {
  qDebug("~Window");
  s_windows.removeOne(this);
}

TabView* Window::activeTabView() {
  if (m_tabViewGroup) {
    return m_tabViewGroup->activeTab();
  } else {
    return nullptr;
  }
}

StatusBar* Window::statusBar() {
  return ui->statusBar;
}

void Window::show() {
  QMainWindow::show();
  QApplication::setActiveWindow(this);
}

QList<Window*> Window::s_windows;

void Window::closeEvent(QCloseEvent* event) {
  qDebug("closeEvent");
  bool isSuccess = m_tabViewGroup->closeAllTabs();
  if (isSuccess) {
    event->accept();
  } else {
    qDebug("closeEvent is ignored");
    event->ignore();
  }
}

bool Window::openDir(const QString& dirPath) {
  if (!m_projectView) {
    m_projectView = new ProjectTreeView(this);
  }

  if (m_projectView->openDirOrExpand(dirPath)) {
    // root splitter becomes the owner of a project view.
    ui->rootSplitter->insertWidget(0, m_projectView);
    ui->rootSplitter->setCollapsible(0, false);
    // Set the initial sizes for QSplitter widgets
    QList<int> sizes;
    sizes << 50 << 300;
    ui->rootSplitter->setSizes(sizes);
    return true;
  } else {
    return false;
  }
}

void Window::openFindAndReplacePanel() {
  m_findReplaceView->show();
}

void Window::paintEvent(QPaintEvent* event) {
  QMainWindow::paintEvent(event);
  if (!m_firstPaintEventFired) {
    m_firstPaintEventFired = true;
    emit firstPaintEventFired();
  }
}

void Window::hideFindReplacePanel() {
  if (m_findReplaceView) {
    m_findReplaceView->hide();
  }
}

QToolBar* Window::findToolbar(const QString& id) {
  return findChild<QToolBar*>(id);
}
