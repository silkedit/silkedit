#include <yaml-cpp/yaml.h>
#include <QFileDialog>
#include <QApplication>
#include <QDebug>
#include <QVBoxLayout>

#include "API.h"
#include "Window.h"
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
      m_rootSplitter(new HSplitter(parent)),
      m_tabViewGroup(new TabViewGroup(this)),
      m_statusBar(new StatusBar(this)),
      m_projectView(nullptr),
      m_findReplaceView(new FindReplaceView(this)) {
  qDebug("creating Window");

  setWindowTitle("SilkEdit");
  setAttribute(Qt::WA_DeleteOnClose);

  // Copy global menu bar
  QList<QAction*> actions = MenuBar::globalMenuBar()->actions();
  menuBar()->insertActions(nullptr, actions);

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

  connect(m_tabViewGroup, &TabViewGroup::activeTabViewChanged, this,
          static_cast<void (Window::*)(TabView*, TabView*)>(&Window::updateConnection));
  connect(m_tabViewGroup, &TabViewGroup::activeTabViewChanged, this,
          &Window::emitActiveEditViewChanged);
  connect(this, &Window::activeEditViewChanged, m_statusBar,
          &StatusBar::onActiveTextEditViewChanged);

  updateConnection(nullptr, m_tabViewGroup->activeTab());
}

void Window::updateConnection(TabView* oldTabView, TabView* newTabView) {
  //  qDebug("updateConnection for new active TabView");

  if (oldTabView && m_statusBar) {
    disconnect(oldTabView, &TabView::activeTextEditViewChanged, m_statusBar,
               &StatusBar::onActiveTextEditViewChanged);
    disconnect(
        oldTabView, &TabView::activeTextEditViewChanged, this,
        static_cast<void (Window::*)(TextEditView*, TextEditView*)>(&Window::updateConnection));
  }

  if (newTabView && m_statusBar) {
    connect(newTabView, &TabView::activeTextEditViewChanged, m_statusBar,
            &StatusBar::onActiveTextEditViewChanged);
    connect(newTabView, &TabView::activeTextEditViewChanged, this,
            static_cast<void (Window::*)(TextEditView*, TextEditView*)>(&Window::updateConnection));
  }
}

void Window::updateConnection(TextEditView* oldEditView, TextEditView* newEditView) {
  //  qDebug("updateConnection for new active TextEditView");

  if (oldEditView && m_statusBar) {
    disconnect(oldEditView, &TextEditView::languageChanged, m_statusBar, &StatusBar::setLanguage);
    disconnect(oldEditView, &TextEditView::encodingChanged, m_statusBar, &StatusBar::setEncoding);
    disconnect(oldEditView, &TextEditView::lineSeparatorChanged, m_statusBar,
               &StatusBar::setLineSeparator);
  }

  if (newEditView && m_statusBar) {
    connect(newEditView, &TextEditView::languageChanged, m_statusBar, &StatusBar::setLanguage);
    connect(newEditView, &TextEditView::encodingChanged, m_statusBar, &StatusBar::setEncoding);
    connect(newEditView, &TextEditView::lineSeparatorChanged, m_statusBar,
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
  window->resize(1280, 720);
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

    YAML::Node menuNode = rootNode["menu"];
    PlatformUtil::parseMenuNode(pkgName, menuNode, s_windows);
  } catch (const YAML::ParserException& ex) {
    qWarning("Unable to load %s. Cause: %s", ymlPath.c_str(), ex.what());
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

void Window::setStatusBar(StatusBar* statusBar) {
  m_statusBar = statusBar;
  QMainWindow::setStatusBar(statusBar);
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
    m_rootSplitter->insertWidget(0, m_projectView);
    // Set the initial sizes for QSplitter widgets
    QList<int> sizes;
    sizes << 50 << 300;
    m_rootSplitter->setSizes(sizes);
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
