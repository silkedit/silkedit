#include <QFileDialog>
#include <QMainWindow>
#include <QApplication>
#include <QDebug>
#include <QBoxLayout>
#include <QLayout>

#include "API.h"
#include "MainWindow.h"
#include "STabWidget.h"
#include "TextEditView.h"
#include "StatusBar.h"

namespace {
QBoxLayout* findItemFromLayout(QBoxLayout* layout, QWidget* item) {
  for (int i = 0; i < layout->count(); i++) {
    QBoxLayout* subLayout = qobject_cast<QBoxLayout*>(layout->itemAt(i)->layout());
    if (subLayout) {
      QBoxLayout* foundLayout = findItemFromLayout(subLayout, item);
      if (foundLayout)
        return foundLayout;
    }
    QWidget* widget = layout->itemAt(i)->widget();
    if (widget && widget == item)
      return layout;
  }

  return nullptr;
}
}

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      m_activeTabWidget(nullptr),
      m_rootLayout(new QBoxLayout(QBoxLayout::LeftToRight)),
      m_statusBar(nullptr) {
  qDebug("creating MainWindow");

  setWindowTitle(QObject::tr("SilkEdit"));

  auto tabWidget = createTabWidget();
  // Note: The ownership of tabWidget is transferred to the layout, and it's the layout's
  // responsibility to delete it.
  m_rootLayout->addWidget(tabWidget);
  m_rootLayout->setContentsMargins(0, 0, 0, 0);
  m_rootLayout->setSpacing(0);
  m_rootLayout->setMargin(0);
  QWidget* window = new QWidget(this);
  // window becomes parent of this layout by setLayout
  window->setLayout(m_rootLayout);
  setCentralWidget(window);

  m_statusBar = new StatusBar(this);
  setStatusBar(m_statusBar);

  setActiveTabWidget(tabWidget);
}

STabWidget* MainWindow::createTabWidget() {
  auto tabWidget = new STabWidget();
  QObject::connect(tabWidget, &STabWidget::allTabRemoved, [this, tabWidget]() {
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

void MainWindow::removeTabWidget(STabWidget* widget) {
  m_tabWidgets.remove(widget);
  // Note: The ownership of widget remains the same as when it was added.
  m_rootLayout->removeWidget(widget);
  widget->deleteLater();
}

MainWindow* MainWindow::create(QWidget* parent, Qt::WindowFlags flags) {
  MainWindow* window = new MainWindow(parent, flags);
  window->resize(1280, 720);
  s_windows.append(window);
  return window;
}

MainWindow::~MainWindow() { qDebug("~MainWindow"); }

void MainWindow::setActiveTabWidget(STabWidget* tabWidget) {
  qDebug("setActiveTabWidget");

  if (m_activeTabWidget && m_activeTabWidget->activeEditView()) {
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

  if (tabWidget) {
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
    deleteLater();
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
  addTabWidget(widget, label, QBoxLayout::LeftToRight, QBoxLayout::TopToBottom);
}

void MainWindow::addTabWidgetVertically(QWidget* widget, const QString& label) {
  addTabWidget(widget, label, QBoxLayout::TopToBottom, QBoxLayout::LeftToRight);
}

void MainWindow::addTabWidget(QWidget* widget,
                              const QString& label,
                              QBoxLayout::Direction activeLayoutDirection,
                              QBoxLayout::Direction newDirection) {
  auto tabWidget = createTabWidget();
  tabWidget->addTab(widget, label);

  STabWidget* activeTabWidget = API::activeTabWidget();
  QBoxLayout* layoutInActiveEditView = findItemFromLayout(m_rootLayout, activeTabWidget);
  if (layoutInActiveEditView->direction() == activeLayoutDirection) {
    int index = layoutInActiveEditView->indexOf(activeTabWidget);
    Q_ASSERT(index >= 0);
    layoutInActiveEditView->removeWidget(activeTabWidget);
    QBoxLayout* layout = new QBoxLayout(newDirection);
    layout->addWidget(activeTabWidget);
    layout->addWidget(tabWidget);
    layoutInActiveEditView->insertLayout(index, layout);
  } else {
    layoutInActiveEditView->addWidget(tabWidget);
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
