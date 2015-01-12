#include <QFileDialog>
#include <QMainWindow>
#include <QApplication>
#include <QDebug>

#include "API.h"
#include "MainWindow.h"
#include "TabWidget.h"
#include "TextEditView.h"
#include "StatusBar.h"
#include "ProjectTreeView.h"
#include "Splitter.h"

namespace {
QSplitter* findItemFromSplitter(QSplitter* splitter, QWidget* item) {
  for (int i = 0; i < splitter->count(); i++) {
    QSplitter* subSplitter = qobject_cast<QSplitter*>(splitter->widget(i));
    if (subSplitter) {
      QSplitter* foundSplitter = findItemFromSplitter(subSplitter, item);
      if (foundSplitter)
        return foundSplitter;
    }
    QWidget* widget = splitter->widget(i);
    if (widget && widget == item)
      return splitter;
  }

  return nullptr;
}
}

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      m_activeTabWidget(nullptr),
      m_rootSplitter(new HSplitter(parent)),
      m_statusBar(nullptr),
      m_projectView(nullptr) {
  qDebug("creating MainWindow");

  setWindowTitle(QObject::tr("SilkEdit"));
  setAttribute(Qt::WA_DeleteOnClose);

  m_rootSplitter->setContentsMargins(0, 0, 0, 0);

  // project tree view
  if (m_projectView) {
    m_rootSplitter->addWidget(m_projectView);
  }

  // Add tab widget
  auto tabWidget = createTabWidget();
  // Note: The ownership of tabWidget is transferred to the splitter, and it's the splitter's
  // responsibility to delete it.
  m_rootSplitter->addWidget(tabWidget);
  setCentralWidget(m_rootSplitter);

  m_statusBar = new StatusBar(this);
  setStatusBar(m_statusBar);

  setActiveTabWidget(tabWidget);
}

TabWidget* MainWindow::createTabWidget() {
  auto tabWidget = new TabWidget();
  QObject::connect(tabWidget, &TabWidget::allTabRemoved, [this, tabWidget]() {
    qDebug() << "allTabRemoved";
    removeTabWidget(tabWidget);

    if (m_tabWidgets.size() == 0) {
      if (tabWidget->tabDragging()) {
        hide();
      } else {
        close();
      }
    }
  });

  m_tabWidgets.push_back(tabWidget);

  return tabWidget;
}

void MainWindow::removeTabWidget(TabWidget* widget) {
  m_tabWidgets.remove(widget);
  widget->hide();
  widget->deleteLater();
}

MainWindow* MainWindow::create(QWidget* parent, Qt::WindowFlags flags) {
  MainWindow* window = new MainWindow(parent, flags);
  window->resize(1280, 720);
  s_windows.append(window);
  return window;
}

MainWindow* MainWindow::createWithNewFile(QWidget* parent, Qt::WindowFlags flags) {
  MainWindow* w = create(parent, flags);
  w->activeTabWidget()->addNew();
  return w;
}

MainWindow::~MainWindow() { qDebug("~MainWindow"); }

void MainWindow::setActiveTabWidget(TabWidget* tabWidget) {
  qDebug("setActiveTabWidget");

  if (m_activeTabWidget && m_activeTabWidget->activeEditView() && m_statusBar) {
    //    qDebug("disconnect previous active tab widget and TextEditView");
    // disconnect from previous active TextEditView
    disconnect(m_activeTabWidget,
               SIGNAL(activeTextEditViewChanged(TextEditView*)),
               m_statusBar,
               SLOT(onActiveTextEditViewChanged(TextEditView*)));
    if (m_activeTabWidget->activeEditView()) {
      disconnect(m_activeTabWidget->activeEditView(),
                 SIGNAL(languageChanged(QString)),
                 m_statusBar,
                 SLOT(setLanguage(QString)));
    }
  }

  m_activeTabWidget = tabWidget;

  if (tabWidget && m_statusBar) {
    // connect to new active TextEditView
    //    qDebug("connect to new active tab widget and activeEditView");
    connect(tabWidget,
            SIGNAL(activeTextEditViewChanged(TextEditView*)),
            m_statusBar,
            SLOT(onActiveTextEditViewChanged(TextEditView*)));
    if (tabWidget->activeEditView()) {
      connect(tabWidget->activeEditView(),
              SIGNAL(languageChanged(const QString&)),
              m_statusBar,
              SLOT(setLanguage(const QString&)));
    }
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

void MainWindow::saveAllTabs() {
  for (auto tabWidget : m_tabWidgets) {
    tabWidget->saveAllTabs();
  }
}

bool MainWindow::closeAllTabs() {
  while (!m_tabWidgets.empty()) {
    bool isSuccess = m_tabWidgets.front()->closeAllTabs();
    if (!isSuccess)
      return false;
  }

  return true;
}

void MainWindow::splitTabHorizontally() {
  splitTab(std::bind(
      &MainWindow::addTabWidgetHorizontally, this, std::placeholders::_1, std::placeholders::_2));
}

void MainWindow::splitTabVertically() {
  splitTab(std::bind(
      &MainWindow::addTabWidgetVertically, this, std::placeholders::_1, std::placeholders::_2));
}

void MainWindow::addTabWidgetHorizontally(QWidget* widget, const QString& label) {
  addTabWidget(widget, label, Qt::Orientation::Horizontal, Qt::Orientation::Vertical);
}

void MainWindow::addTabWidgetVertically(QWidget* widget, const QString& label) {
  addTabWidget(widget, label, Qt::Orientation::Vertical, Qt::Orientation::Horizontal);
}

void MainWindow::addTabWidget(QWidget* widget,
                              const QString& label,
                              Qt::Orientation activeSplitterDirection,
                              Qt::Orientation newDirection) {
  auto tabWidget = createTabWidget();
  tabWidget->addTab(widget, label);

  TabWidget* activeTabWidget = API::activeTabWidget();
  QSplitter* splitterInActiveEditView = findItemFromSplitter(m_rootSplitter, activeTabWidget);
  if (splitterInActiveEditView->orientation() == activeSplitterDirection) {
    int index = splitterInActiveEditView->indexOf(activeTabWidget);
    Q_ASSERT(index >= 0);
    Splitter* splitter = new Splitter(newDirection);
    splitter->addWidget(activeTabWidget);
    splitter->addWidget(tabWidget);
    splitterInActiveEditView->insertWidget(index, splitter);
  } else {
    splitterInActiveEditView->addWidget(tabWidget);
  }
}

void MainWindow::splitTab(std::function<void(QWidget*, const QString&)> func) {
  if (m_activeTabWidget) {
    TextEditView* activeEditView = m_activeTabWidget->activeEditView();
    QString label = m_activeTabWidget->tabText(m_activeTabWidget->currentIndex());
    if (activeEditView) {
      TextEditView* anotherEditView = activeEditView->clone();
      func(anotherEditView, label);
    }
  }
}

QList<MainWindow*> MainWindow::s_windows;

void MainWindow::closeEvent(QCloseEvent* event) {
  qDebug("closeEvent");
  bool isSuccess = closeAllTabs();
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
