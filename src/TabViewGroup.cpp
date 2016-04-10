#include <QDebug>
#include <QVBoxLayout>
#include <QTabBar>
#include <QEventLoop>
#include <QApplication>

#include "TabViewGroup.h"
#include "TabView.h"
#include "Splitter.h"
#include "TextEdit.h"
#include "TabBar.h"
#include "Window.h"
#include "core/scoped_guard.h"

using core::scoped_guard;

namespace {

const QString& TABS_PREFIX = QStringLiteral("tabs");
const QString& ORIENTATION_PREFIX = QStringLiteral("orientation");
const QString& WIDGETS_PREFIX = QStringLiteral("widgets");
const QString& SPLITTER_PREFIX = QStringLiteral("Splitter");

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
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_rootSplitter);
  setLayout(layout);
}

TabView* TabViewGroup::activeTab() {
  if (m_activeTabView) {
    return m_activeTabView;
  }
  return nullptr;
}

void TabViewGroup::emitCurrentChanged(int index) {
  emit currentViewChanged(m_activeTabView->widget(index));
}

void TabViewGroup::setActiveTab(TabView* newTabView) {
  if (m_activeTabView != newTabView) {
    TabView* oldtabView = m_activeTabView;
    m_activeTabView = newTabView;
    if (oldtabView) {
      disconnect(oldtabView, &TabView::currentChanged, this, &TabViewGroup::emitCurrentChanged);
    }
    connect(m_activeTabView, &TabView::currentChanged, this, &TabViewGroup::emitCurrentChanged);
    emit activeTabViewChanged(oldtabView, newTabView);
    emitCurrentChanged(newTabView->currentIndex());
  }
}

bool TabViewGroup::closeAllTabs() {
  QVector<TabView*> tabs = tabViews();
  while (!tabs.empty()) {
    bool isSuccess = tabs.front()->closeAllTabs();
    if (!isSuccess) {
      return false;
    }
    tabs.removeFirst();
  }

  return true;
}

void TabViewGroup::splitHorizontally() {
  splitTextEdit(
      std::bind(static_cast<void (TabViewGroup::*)(QWidget* initialWidget, const QString& label)>(
                    &TabViewGroup::splitHorizontally),
                this, std::placeholders::_1, std::placeholders::_2));
}

void TabViewGroup::splitVertically() {
  splitTextEdit(
      std::bind(static_cast<void (TabViewGroup::*)(QWidget* initialWidget, const QString& label)>(
                    &TabViewGroup::splitVertically),
                this, std::placeholders::_1, std::placeholders::_2));
}

TabBar* TabViewGroup::tabBarAt(int screenX, int screenY) {
  for (TabView* tabView : tabViews()) {
    QRegion region = tabView->tabBar()->visibleRegion();
    if (region.contains(tabView->tabBar()->mapFromGlobal(QPoint(screenX, screenY)))) {
      return qobject_cast<TabBar*>(tabView->tabBar());
    }
  }

  return nullptr;
}

QVector<TabView*> TabViewGroup::tabViews() {
  Q_ASSERT(m_rootSplitter);
  return tabViews(m_rootSplitter);
}

QVector<TabView*> TabViewGroup::tabViews(QSplitter* splitter) {
  QVector<TabView*> tabs;

  for (int i = 0; i < splitter->count(); i++) {
    if (auto tab = qobject_cast<TabView*>(splitter->widget(i))) {
      if (tab->isVisible()) {
        tabs.append(tab);
      }
    } else if (auto childSplitter = qobject_cast<QSplitter*>(splitter->widget(i))) {
      tabs.append(tabViews(childSplitter));
    } else {
      qWarning() << "widget(" << i << ") is neither TabView nor Splitter";
    }
  }

  return tabs;
}

void TabViewGroup::saveState(QSettings& settings) {
  settings.beginGroup(TabViewGroup::staticMetaObject.className());
  scoped_guard guard([&] { settings.endGroup(); });

  Q_ASSERT(m_rootSplitter);
  saveState(m_rootSplitter, settings);
}

void TabViewGroup::loadState(QSettings& settings) {
  settings.beginGroup(TabViewGroup::staticMetaObject.className());
  scoped_guard guard([&] { settings.endGroup(); });

  Q_ASSERT(m_rootSplitter);
  loadState(m_rootSplitter, settings);

  // restore active tabView
  for (int i = 0; i < m_rootSplitter->count(); i++) {
    if (auto tabView = qobject_cast<TabView*>(m_rootSplitter->widget(i))) {
      setActiveTab(tabView);
      break;
    }
  }
}

void TabViewGroup::saveState(QSplitter* splitter, QSettings& settings) {
  settings.beginGroup(SPLITTER_PREFIX);
  scoped_guard guard([&] { settings.endGroup(); });

  settings.setValue(ORIENTATION_PREFIX, splitter->orientation());

  settings.beginWriteArray(WIDGETS_PREFIX);
  int arrayIndex = 0;
  for (int i = 0; i < splitter->count(); i++) {
    if (TabView* tab = qobject_cast<TabView*>(splitter->widget(i))) {
      if (tab->canSave()) {
        settings.setArrayIndex(arrayIndex);
        tab->saveState(settings);
        arrayIndex++;
      }
    } else if (QSplitter* childSplitter = qobject_cast<QSplitter*>(splitter->widget(i))) {
      settings.setArrayIndex(arrayIndex);
      saveState(childSplitter, settings);
      arrayIndex++;
    } else {
      qWarning() << "widget(" << i << ") is neither TabView nor Splitter";
    }
  }
  settings.endArray();
}

void TabViewGroup::loadState(QSplitter* splitter, QSettings& settings) {
  Q_ASSERT(splitter);

  settings.beginGroup(SPLITTER_PREFIX);

  if (settings.contains(ORIENTATION_PREFIX)) {
    auto orientationVar = settings.value(ORIENTATION_PREFIX);
    if (orientationVar.canConvert<int>()) {
      splitter->setOrientation(static_cast<Qt::Orientation>(orientationVar.toInt()));
    }
  }

  int size = settings.beginReadArray(WIDGETS_PREFIX);
  scoped_guard guard([&] {
    settings.endArray();
    settings.endGroup();
  });

  for (int i = 0; i < size; i++) {
    settings.setArrayIndex(i);

    if (settings.childGroups().contains(TabView::SETTINGS_PREFIX)) {
      auto tab = createTabView();
      if (tab) {
        tab->loadState(settings);
        splitter->addWidget(tab);
      }
    } else if (settings.childGroups().contains(SPLITTER_PREFIX)) {
      auto childSplitter = new QSplitter(splitter);
      if (childSplitter) {
        loadState(childSplitter, settings);
        splitter->addWidget(childSplitter);
      }
    } else {
      qWarning() << "widget is neither TabView nor Splitter";
    }
  }
}

TabView* TabViewGroup::createTabView() {
  auto tabView = new TabView();
  QObject::connect(tabView, &TabView::allTabRemoved, [this, tabView]() {
    qDebug() << "allTabRemoved";

    if (tabView == m_activeTabView) {
      m_activeTabView = nullptr;
    }

    auto tabs = tabViews();
    bool ok = tabs.removeOne(tabView);
    if (ok) {
      tabView->hide();
      tabView->deleteLater();
      QApplication::processEvents();
    } else {
      qWarning() << "tabView is not a child";
    }

    // Set focus to another tabView's active tab
    if (!tabs.isEmpty() && tabs.first()->currentWidget()) {
      tabs.first()->currentWidget()->setFocus();
    }

    Window* win = qobject_cast<Window*>(window());
    if (win && tabs.empty() && !win->isProjectOpend()) {
      win->close();
    }
  });

  return tabView;
}

TabView* TabViewGroup::addNewTabView() {
  auto tabView = createTabView();
  // Note: The ownership of tabView is transferred to the splitter, and it's the splitter's
  // responsibility to delete it.
  m_rootSplitter->addWidget(tabView);
  setActiveTab(tabView);
  return tabView;
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

void TabViewGroup::splitTextEdit(std::function<void(QWidget*, const QString&)> func) {
  if (m_activeTabView) {
    TextEdit* activeTextEdit = qobject_cast<TextEdit*>(m_activeTabView->activeView());
    if (activeTextEdit) {
      TextEdit* anotherEditView = activeTextEdit->clone();
      QString label = m_activeTabView->tabText(m_activeTabView->currentIndex());
      func(anotherEditView, label);
    }
  }
}
