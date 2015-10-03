#include <yaml-cpp/yaml.h>
#include <QFileDialog>
#include <QApplication>
#include <QDebug>
#include <QVBoxLayout>
#include <QToolBar>

#include "Window.h"
#include "ui_Window.h"
#include "API.h"
#include "TabViewGroup.h"
#include "TextEditView.h"
#include "StatusBar.h"
#include "ProjectTreeView.h"
#include "Splitter.h"
#include "TabView.h"
#include "FindReplaceView.h"
#include "MenuBar.h"
#include "CommandAction.h"
#include "util/YamlUtils.h"
#include "Context.h"
#include "PluginManager.h"
#include "PlatformUtil.h"

Window::Window(QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      ui(new Ui::Window),
      m_tabViewGroup(new TabViewGroup(this)),
      m_projectView(nullptr),
      m_findReplaceView(new FindReplaceView(this)) {
  qDebug("creating Window");
  ui->setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);

  // Copy global menu bar
  QList<QAction*> actions = MenuBar::globalMenuBar()->actions();
  menuBar()->insertActions(nullptr, actions);

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

  ui->rootSplitter->addWidget(editorWidget);

  connect(m_tabViewGroup, &TabViewGroup::activeTabViewChanged, this,
          static_cast<void (Window::*)(TabView*, TabView*)>(&Window::updateConnection));
  connect(m_tabViewGroup, &TabViewGroup::activeTabViewChanged, this,
          &Window::emitActiveEditViewChanged);
  connect(this, &Window::activeEditViewChanged, ui->statusBar,
          &StatusBar::onActiveTextEditViewChanged);

  updateConnection(nullptr, m_tabViewGroup->activeTab());
}

void Window::updateConnection(TabView* oldTabView, TabView* newTabView) {
  //  qDebug("updateConnection for new active TabView");

  if (oldTabView && ui->statusBar) {
    disconnect(oldTabView, &TabView::activeTextEditViewChanged, ui->statusBar,
               &StatusBar::onActiveTextEditViewChanged);
    disconnect(
        oldTabView, &TabView::activeTextEditViewChanged, this,
        static_cast<void (Window::*)(TextEditView*, TextEditView*)>(&Window::updateConnection));
  }

  if (newTabView && ui->statusBar) {
    connect(newTabView, &TabView::activeTextEditViewChanged, ui->statusBar,
            &StatusBar::onActiveTextEditViewChanged);
    connect(newTabView, &TabView::activeTextEditViewChanged, this,
            static_cast<void (Window::*)(TextEditView*, TextEditView*)>(&Window::updateConnection));
  }
}

void Window::updateConnection(TextEditView* oldEditView, TextEditView* newEditView) {
  //  qDebug("updateConnection for new active TextEditView");

  if (oldEditView && ui->statusBar) {
    disconnect(oldEditView, &TextEditView::languageChanged, ui->statusBar, &StatusBar::setLanguage);
    disconnect(oldEditView, &TextEditView::encodingChanged, ui->statusBar, &StatusBar::setEncoding);
    disconnect(oldEditView, &TextEditView::lineSeparatorChanged, ui->statusBar,
               &StatusBar::setLineSeparator);
  }

  if (newEditView && ui->statusBar) {
    connect(newEditView, &TextEditView::languageChanged, ui->statusBar, &StatusBar::setLanguage);
    connect(newEditView, &TextEditView::encodingChanged, ui->statusBar, &StatusBar::setEncoding);
    connect(newEditView, &TextEditView::lineSeparatorChanged, ui->statusBar,
            &StatusBar::setLineSeparator);
  }
}

void Window::emitActiveEditViewChanged(TabView* oldTabView, TabView* newTabView) {
  TextEditView* oldEditView = oldTabView ? oldTabView->activeEditView() : nullptr;
  TextEditView* newEditView = newTabView ? newTabView->activeEditView() : nullptr;
  emit activeEditViewChanged(oldEditView, newEditView);
}

Window* Window::create(QWidget* parent, Qt::WindowFlags flags) {
  Window* window = new Window(parent, flags);
  s_windows.append(window);
  return window;
}

Window* Window::createWithNewFile(QWidget* parent, Qt::WindowFlags flags) {
  Window* w = create(parent, flags);
  w->activeTabView()->addNew();
  return w;
}

void Window::loadMenu(const std::string& pkgName, const std::string& ymlPath) {
  qDebug("Start loading. pkg: %s, path: %s", pkgName.c_str(), ymlPath.c_str());
  try {
    YAML::Node rootNode = YAML::LoadFile(ymlPath);
    if (!rootNode.IsMap()) {
      qWarning("root node must be a map");
      return;
    }

    YAML::Node menusNode = rootNode["menus"];
#ifdef Q_OS_MAC
    PlatformUtil::parseMenusNode(pkgName, menusNode);
#elif defined Q_OS_WIN
    PlatformUtil::parseMenuNode(pkgName, menuNode, s_windows);
#endif
  } catch (const YAML::ParserException& ex) {
    qWarning("Unable to load %s. Cause: %s", ymlPath.c_str(), ex.what());
  }
}

void Window::loadToolbar(const std::string& pkgName, const std::string& ymlPath) {
  qDebug("Start loading. pkg: %s, path: %s", pkgName.c_str(), ymlPath.c_str());
  foreach (Window* win, s_windows) {
    try {
      YAML::Node rootNode = YAML::LoadFile(ymlPath);
      if (!rootNode.IsMap()) {
        qWarning("root node must be a map");
        return;
      }

      YAML::Node toolbarsNode = rootNode["toolbars"];
      YamlUtils::parseToolbarsNode(pkgName, ymlPath, win, toolbarsNode);
    } catch (const YAML::ParserException& ex) {
      qWarning("Unable to load %s. Cause: %s", ymlPath.c_str(), ex.what());
    }
  }
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

  if (m_projectView->open(dirPath)) {
    // root splitter becomes the owner of a project view.
    ui->rootSplitter->insertWidget(0, m_projectView);
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

void Window::hideFindReplacePanel() {
  if (m_findReplaceView) {
    m_findReplaceView->hide();
  }
}

QToolBar* Window::findToolbar(const QString& id) {
  if (QToolBar* toolbar = findChild<QToolBar*>(id)) {
    return toolbar;
  } else {
    return nullptr;
  }
}

void Window::request(Window* window,
                     const QString& method,
                     msgpack::rpc::msgid_t msgId,
                     const msgpack::object&) {
  if (method == "statusBar") {
    PluginManager::singleton().sendResponse(window->statusBar()->id(), msgpack::type::nil(), msgId);
  } else {
    qWarning("%s is not supported", qPrintable(method));
    PluginManager::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
  }
}

void Window::notify(Window* window, const QString& method, const msgpack::object&) {
  if (method == "close") {
    window->close();
  } else if (method == "openFindPanel") {
    window->openFindAndReplacePanel();
  }
}
