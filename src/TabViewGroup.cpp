#include <QDebug>
#include <QVBoxLayout>
#include <QTabBar>

#include "TabViewGroup.h"
#include "TabView.h"
#include "Splitter.h"
#include "TextEditView.h"
#include "TabBar.h"
#include "Window.h"

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

void setSizesOfSplitter(QSplitter* splitter) {
  QList<int> sizes;
  for (int i = 0; i < splitter->count(); i++) {
    sizes.append(100);
  }
  splitter->setSizes(sizes);
}
}

TabViewGroup::TabViewGroup(QWidget* parent)
    : QWidget(parent), m_activeTabView(nullptr), m_rootSplitter(new HSplitter(this)) {
  createInitialTabView();
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_rootSplitter);
  setLayout(layout);
}

TabView* TabViewGroup::activeTab() {
  if (m_activeTabView) {
    return m_activeTabView;
  }
  return createInitialTabView();
}

void TabViewGroup::setActiveTab(TabView* tabView) {
  if (m_activeTabView != tabView) {
    TabView* oldtabView = m_activeTabView;
    m_activeTabView = tabView;
    emit activeTabViewChanged(oldtabView, tabView);
  }
}

bool TabViewGroup::closeAllTabs() {
  while (!m_tabViews.empty()) {
    bool isSuccess = m_tabViews.front()->closeAllTabs();
    if (!isSuccess) {
      return false;
    }
  }

  return true;
}

void TabViewGroup::splitHorizontally() {
  splitTextEditView(
      std::bind(static_cast<void (TabViewGroup::*)(QWidget* initialWidget, const QString& label)>(
                    &TabViewGroup::splitHorizontally),
                this, std::placeholders::_1, std::placeholders::_2));
}

void TabViewGroup::splitVertically() {
  splitTextEditView(
      std::bind(static_cast<void (TabViewGroup::*)(QWidget* initialWidget, const QString& label)>(
                    &TabViewGroup::splitVertically),
                this, std::placeholders::_1, std::placeholders::_2));
}

TabBar* TabViewGroup::tabBarAt(int screenX, int screenY) {
  for (TabView* tabView : m_tabViews) {
    QRegion region = tabView->tabBar()->visibleRegion();
    if (region.contains(tabView->tabBar()->mapFromGlobal(QPoint(screenX, screenY)))) {
      return qobject_cast<TabBar*>(tabView->tabBar());
    }
  }

  return nullptr;
}

TabView* TabViewGroup::createTabView() {
  auto tabView = new TabView();
  QObject::connect(tabView, &TabView::allTabRemoved, [this, tabView]() {
    qDebug() << "allTabRemoved";
    removeTabView(tabView);

    Window* win = qobject_cast<Window*>(window());
    if (win && m_tabViews.empty() && !win->isProjectOpend()) {
      win->close();
    }
  });

  m_tabViews.push_back(tabView);

  return tabView;
}

TabView* TabViewGroup::createInitialTabView() {
  auto tabView = createTabView();
  // Note: The ownership of tabView is transferred to the splitter, and it's the splitter's
  // responsibility to delete it.
  m_rootSplitter->addWidget(tabView);
  setActiveTab(tabView);
  return tabView;
}

void TabViewGroup::removeTabView(TabView* widget) {
  if (widget == m_activeTabView) {
    m_activeTabView = nullptr;
  }

  bool result = m_tabViews.removeOne(widget);
  if (result) {
    widget->hide();
    widget->deleteLater();
  } else {
    qWarning() << "widget is not a child";
  }
}

void TabViewGroup::splitHorizontally(QWidget* initialWidget, const QString& label) {
  addTabView(initialWidget, label, Qt::Orientation::Horizontal, Qt::Orientation::Vertical);
}

void TabViewGroup::splitVertically(QWidget* initialWidget, const QString& label) {
  addTabView(initialWidget, label, Qt::Orientation::Vertical, Qt::Orientation::Horizontal);
}

void TabViewGroup::addTabView(QWidget* widget,
                              const QString& label,
                              Qt::Orientation activeSplitterDirection,
                              Qt::Orientation newDirection) {
  auto tabView = createTabView();
  tabView->addTab(widget, label);

  QSplitter* splitterInActiveTabViewGroup = findItemFromSplitter(m_rootSplitter, m_activeTabView);
  if (splitterInActiveTabViewGroup->orientation() == activeSplitterDirection) {
    int index = splitterInActiveTabViewGroup->indexOf(m_activeTabView);
    Q_ASSERT(index >= 0);
    Splitter* splitter = new Splitter(newDirection);
    splitter->addWidget(m_activeTabView);
    splitter->addWidget(tabView);
    setSizesOfSplitter(splitter);
    splitterInActiveTabViewGroup->insertWidget(index, splitter);
    setSizesOfSplitter(splitterInActiveTabViewGroup);
  } else {
    splitterInActiveTabViewGroup->addWidget(tabView);
    setSizesOfSplitter(splitterInActiveTabViewGroup);
  }
}

void TabViewGroup::splitTextEditView(std::function<void(QWidget*, const QString&)> func) {
  if (m_activeTabView) {
    TextEditView* activeEditView = qobject_cast<TextEditView*>(m_activeTabView->activeView());
    if (activeEditView) {
      TextEditView* anotherEditView = activeEditView->clone();
      QString label = m_activeTabView->tabText(m_activeTabView->currentIndex());
      func(anotherEditView, label);
    }
  }
}
