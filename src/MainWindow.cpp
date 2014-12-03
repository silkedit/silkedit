#include <QFileDialog>
#include <QMainWindow>
#include <QApplication>
#include <QDebug>
#include <QBoxLayout>

#include "API.h"
#include "MainWindow.h"
#include "STabWidget.h"
#include "TextEditView.h"

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      m_activeTabWidget(nullptr),
      m_layout(new QBoxLayout(QBoxLayout::LeftToRight)) {
  qDebug("creating MainWindow");

  setWindowTitle(QObject::tr("SilkEdit"));

  auto tabWidget = createTabWidget();
  // Note: The ownership of tabWidget is transferred to the layout, and it's the layout's responsibility to delete it.
  m_layout->addWidget(tabWidget);
  m_layout->setContentsMargins(0, 0, 0, 0);
  m_layout->setSpacing(0);
  m_layout->setMargin(0);
  QWidget* window = new QWidget(this);
  // window becomes parent of this layout by setLayout
  window->setLayout(m_layout);
  setCentralWidget(window);
  m_activeTabWidget = tabWidget;
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

  m_tabWidgets.append(tabWidget);

  return tabWidget;
}

void MainWindow::removeTabWidget(STabWidget *widget)
{
    m_tabWidgets.removeOne(widget);
    // Note: The ownership of widget remains the same as when it was added.
    m_layout->removeWidget(widget);
    widget->deleteLater();
}

MainWindow* MainWindow::create(QWidget* parent, Qt::WindowFlags flags) {
  MainWindow* window = new MainWindow(parent, flags);
  window->resize(1280, 720);
  s_windows.append(window);
  return window;
}

MainWindow::~MainWindow() {
  qDebug("~MainWindow");
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

void MainWindow::splitTabHorizontally() {
  if (m_activeTabWidget) {
    TextEditView* activeEditView = m_activeTabWidget->activeEditView();
    QString label = m_activeTabWidget->tabText(m_activeTabWidget->currentIndex());
    if (activeEditView) {
      TextEditView* anotherEditView = activeEditView->clone();
      addTabWidgetHorizontally(anotherEditView, label);
    }
  }
}

void MainWindow::addTabWidgetHorizontally(QWidget* widget, const QString& label) {
  auto tabWidget = createTabWidget();
  if (m_layout->direction() == QBoxLayout::LeftToRight) {
    tabWidget->addTab(widget, label);
    m_layout->addWidget(tabWidget);
  }
}

QList<MainWindow*> MainWindow::s_windows;
