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
      m_rootLayout(new QBoxLayout(QBoxLayout::LeftToRight)) {
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
  m_activeTabWidget = tabWidget;

  StatusBar* sbar = new StatusBar(this);
  connect(
      this, &MainWindow::activeTextEditViewChanged, sbar, &StatusBar::onActiveTextEditViewChanged);
  setStatusBar(sbar);
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

void MainWindow::closeAllTabs() {
  for (auto tabWidget : m_tabWidgets) {
    tabWidget->closeAllTabs();
  }
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
