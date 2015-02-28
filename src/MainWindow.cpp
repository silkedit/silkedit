#include <yaml-cpp/yaml.h>
#include <QFileDialog>
#include <QMainWindow>
#include <QApplication>
#include <QDebug>
#include <QVBoxLayout>

#include "API.h"
#include "MainWindow.h"
#include "TabViewGroup.h"
#include "TextEditView.h"
#include "StatusBar.h"
#include "ProjectTreeView.h"
#include "Splitter.h"
#include "TabView.h"
#include "FindReplaceView.h"
#include "MenuBar.h"
#include "CommandAction.h"

namespace {
QAction* findAction(QList<QAction*> actions, const QString& label) {
  foreach (QAction* action, actions) {
    if (action->text().replace("&", "") == label) {
      return action;
    }
  }

  return nullptr;
}

void parseMenuNode(QWidget* parent, YAML::Node menuNode) {
  if (!menuNode.IsSequence()) {
    qWarning("menuNode must be a sequence.");
    return;
  }

  for (auto it = menuNode.begin(); it != menuNode.end(); it++) {
    YAML::Node node = *it;
    if (!node.IsMap()) {
      qWarning("menu item must be a map");
      continue;
    }

    YAML::Node labelNode = node["label"];
    if (labelNode.IsDefined() && labelNode.IsScalar()) {
      QString label = QString::fromUtf8(labelNode.as<std::string>().c_str());
      YAML::Node commandNode = node["command"];
      YAML::Node submenuNode = node["submenu"];
      if (submenuNode.IsDefined()) {
        QMenu* currentMenu;
        if (QAction* action = findAction(parent->actions(), label)) {
          currentMenu = action->menu();
        } else {
          currentMenu = new QMenu(label, parent);
          if (QMenuBar* menuBar = qobject_cast<QMenuBar*>(parent)) {
            menuBar->addMenu(currentMenu);
          } else if (QMenu* parentMenu = qobject_cast<QMenu*>(parent)) {
            parentMenu->addMenu(currentMenu);
          }
        }
        parseMenuNode(currentMenu, submenuNode);
      } else if (commandNode.IsDefined()) {
        QString command = QString::fromUtf8(commandNode.as<std::string>().c_str());
        auto commandAction = new CommandAction(label, command);
        if (!findAction(parent->actions(), label)) {
          if (QMenuBar* menuBar = qobject_cast<QMenuBar*>(parent)) {
            menuBar->addAction(commandAction);
          } else if (QMenu* parentMenu = qobject_cast<QMenu*>(parent)) {
            parentMenu->addAction(commandAction);
          }
        } else {
          qWarning("%s already exists", qPrintable(label));
        }
      }
    }
  }
}
}

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      m_rootSplitter(new HSplitter(parent)),
      m_tabViewGroup(new TabViewGroup(this)),
      m_statusBar(new StatusBar(this)),
      m_projectView(nullptr),
      m_findReplaceView(new FindReplaceView(this)) {
  qDebug("creating MainWindow");

  setWindowTitle(QObject::tr("SilkEdit"));
  setAttribute(Qt::WA_DeleteOnClose);

  // MainWindow takes ownership of the menuBar pointer and deletes it at the appropriate time.
  setMenuBar(new MenuBar);

  m_rootSplitter->setContentsMargins(0, 0, 0, 0);

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

  m_rootSplitter->addWidget(editorWidget);
  setCentralWidget(m_rootSplitter);

  setStatusBar(m_statusBar);

  connect(m_tabViewGroup,
          &TabViewGroup::activeTabViewChanged,
          this,
          static_cast<void (MainWindow::*)(TabView*, TabView*)>(&MainWindow::updateConnection));
  connect(m_tabViewGroup,
          &TabViewGroup::activeTabViewChanged,
          this,
          &MainWindow::emitActiveEditViewChanged);
  connect(this,
          &MainWindow::activeEditViewChanged,
          m_statusBar,
          &StatusBar::onActiveTextEditViewChanged);

  updateConnection(nullptr, m_tabViewGroup->activeTab());
}

void MainWindow::updateConnection(TabView* oldTabView, TabView* newTabView) {
  qDebug("updateConnection for new active TabView");

  if (oldTabView && m_statusBar) {
    disconnect(oldTabView,
               &TabView::activeTextEditViewChanged,
               m_statusBar,
               &StatusBar::onActiveTextEditViewChanged);
    disconnect(oldTabView,
               &TabView::activeTextEditViewChanged,
               this,
               static_cast<void (MainWindow::*)(TextEditView*, TextEditView*)>(
                   &MainWindow::updateConnection));
  }

  if (newTabView && m_statusBar) {
    connect(newTabView,
            &TabView::activeTextEditViewChanged,
            m_statusBar,
            &StatusBar::onActiveTextEditViewChanged);
    connect(newTabView,
            &TabView::activeTextEditViewChanged,
            this,
            static_cast<void (MainWindow::*)(TextEditView*, TextEditView*)>(
                &MainWindow::updateConnection));
  }
}

void MainWindow::updateConnection(TextEditView* oldEditView, TextEditView* newEditView) {
  qDebug("updateConnection for new active TextEditView");

  if (oldEditView && m_statusBar) {
    disconnect(
        oldEditView, SIGNAL(languageChanged(QString)), m_statusBar, SLOT(setLanguage(QString)));
  }

  if (newEditView && m_statusBar) {
    connect(newEditView,
            SIGNAL(languageChanged(const QString&)),
            m_statusBar,
            SLOT(setLanguage(const QString&)));
  }
}

void MainWindow::emitActiveEditViewChanged(TabView* oldTabView, TabView* newTabView) {
  TextEditView* oldEditView = oldTabView ? oldTabView->activeEditView() : nullptr;
  TextEditView* newEditView = newTabView ? newTabView->activeEditView() : nullptr;
  emit activeEditViewChanged(oldEditView, newEditView);
}

MainWindow* MainWindow::create(QWidget* parent, Qt::WindowFlags flags) {
  MainWindow* window = new MainWindow(parent, flags);
  window->resize(1280, 720);
  s_windows.append(window);
  return window;
}

MainWindow* MainWindow::createWithNewFile(QWidget* parent, Qt::WindowFlags flags) {
  MainWindow* w = create(parent, flags);
  w->activeTabView()->addNew();
  return w;
}

void MainWindow::loadMenu(const std::string& ymlPath) {
  qDebug("Start loading: %s", ymlPath.c_str());
  try {
    YAML::Node rootNode = YAML::LoadFile(ymlPath);
    if (!rootNode.IsMap()) {
      qWarning("root node must be a map");
      return;
    }

    YAML::Node menuNode = rootNode["menu"];
    foreach (MainWindow* win, s_windows) { parseMenuNode(win->menuBar(), menuNode); }
  } catch (const YAML::ParserException& ex) {
    qWarning("Unable to load %s. Cause: %s", ymlPath.c_str(), ex.what());
  }
}

MainWindow::~MainWindow() {
  qDebug("~MainWindow");
}

TabView* MainWindow::activeTabView() {
  if (m_tabViewGroup) {
    return m_tabViewGroup->activeTab();
  } else {
    return nullptr;
  }
}

void MainWindow::show() {
  QMainWindow::show();
  QApplication::setActiveWindow(this);
}

void MainWindow::close() {
  if (s_windows.removeOne(this)) {
    QMainWindow::close();
  }
}

QList<MainWindow*> MainWindow::s_windows;

void MainWindow::closeEvent(QCloseEvent* event) {
  qDebug("closeEvent");
  bool isSuccess = m_tabViewGroup->closeAllTabs();
  if (isSuccess) {
    event->accept();
  } else {
    qDebug("closeEvent is ignored");
    event->ignore();
  }
}

bool MainWindow::openDir(const QString& dirPath) {
  if (!m_projectView) {
    m_projectView = new ProjectTreeView(this);
  }

  if (m_projectView->open(dirPath)) {
    // root splitter becomes the owner of a project view.
    m_rootSplitter->insertWidget(0, m_projectView);
    return true;
  } else {
    return false;
  }
}

void MainWindow::openFindAndReplacePanel() {
  m_findReplaceView->show();
}

void MainWindow::hideFindReplacePanel() {
  if (m_findReplaceView) {
    m_findReplaceView->hide();
  }
}
