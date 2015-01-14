#include <memory>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

#include "TabView.h"
#include "TextEditView.h"
#include "KeymapService.h"
#include "TabBar.h"
#include "MainWindow.h"
#include "DraggingTabInfo.h"
#include "SilkApp.h"
#include "DocumentService.h"

namespace {
QString getFileNameFrom(const QString& path) {
  QFileInfo info(path);
  return info.fileName();
}
}

TabView::TabView(QWidget* parent)
    : QTabWidget(parent),
      m_activeEditView(nullptr),
      m_tabBar(new TabBar(this)),
      m_tabDragging(false) {
  connect(m_tabBar,
          SIGNAL(onDetachTabStarted(int, const QPoint&)),
          this,
          SLOT(detachTabStarted(int, const QPoint&)));

  connect(m_tabBar,
          SIGNAL(onDetachTabEntered(const QPoint&)),
          this,
          SLOT(detachTabEntered(const QPoint&)));

  connect(m_tabBar,
          SIGNAL(onDetachTabFinished(const QPoint&)),
          this,
          SLOT(detachTabFinished(const QPoint&)));

  setTabBar(m_tabBar);
  setMovable(true);
  setDocumentMode(true);
  setTabsClosable(true);
  setTabShape(TabShape::Triangular);

  QObject::connect(this, &QTabWidget::currentChanged, [this](int index) {
    // This lambda is called after m_tabbar is deleted when shutdown.
    if (index < 0)
      return;

    qDebug("currentChanged. index: %i, tab count: %i", index, count());
    if (auto w = widget(index)) {
      setActiveEditView(qobject_cast<TextEditView*>(w));
    } else {
      qDebug("active edit view is null");
      setActiveEditView(nullptr);
    }
  });

  QObject::connect(this, &QTabWidget::tabCloseRequested, [this](int index) {
    qDebug("tab widget (index %i) is deleted", index);
    removeTabAndWidget(index);
  });
}

TabView::~TabView() {
  qDebug("~TabView");
  disconnect(m_tabBar,
             SIGNAL(onDetachTabStarted(int, const QPoint&)),
             this,
             SLOT(detachTabStarted(int, const QPoint&)));

  disconnect(m_tabBar,
             SIGNAL(onDetachTabEntered(const QPoint&)),
             this,
             SLOT(detachTabEntered(const QPoint&)));

  disconnect(m_tabBar,
             SIGNAL(onDetachTabFinished(const QPoint&)),
             this,
             SLOT(detachTabFinished(const QPoint&)));
}

int TabView::addTab(QWidget* page, const QString& label) { return insertTab(-1, page, label); }

int TabView::insertTab(int index, QWidget* w, const QString& label) {
  w->setParent(this);
  TextEditView* editView = qobject_cast<TextEditView*>(w);
  if (editView) {
    QObject::connect(editView, &TextEditView::pathUpdated, [this, editView](const QString& path) {
      setTabText(indexOf(editView), getFileNameFrom(path));
    });
    QObject::connect(
        editView, &TextEditView::saved, [editView]() { editView->document()->setModified(false); });
    connect(editView, SIGNAL(modificationChanged(bool)), this, SLOT(updateTabTextBasedOn(bool)));
  } else {
    qDebug("inserted widget is not TextEditView");
  }
  bool result = QTabWidget::insertTab(index, w, label);
  if (count() == 1 && result) {
    m_activeEditView = editView;
  }
}

int TabView::open(const QString& path) {
  qDebug("TabView::open(%s)", qPrintable(path));
  int index = indexOfPath(path);
  if (index >= 0) {
    setCurrentIndex(index);
    return index;
  }

  std::shared_ptr<Document> newDoc(Document::create(path));
  if (!newDoc) {
    return -1;
  }
  newDoc->setModified(false);

  if (count() == 1) {
    TextEditView* editView = qobject_cast<TextEditView*>(currentWidget());
    if (editView && !editView->document()->isModified() && editView->document()->isEmpty()) {
      qDebug("trying to replace am empty doc with a new one");
      editView->setDocument(std::move(newDoc));
      editView->setPath(path);
      return currentIndex();
    }
  }

  TextEditView* view = new TextEditView(this);
  view->setDocument(std::move(newDoc));
  return addTab(view, getFileNameFrom(path));
}

void TabView::addNew() {
  TextEditView* view = new TextEditView(this);
  std::shared_ptr<Document> newDoc(Document::createBlank());
  view->setDocument(std::move(newDoc));
  addTab(view, DocumentService::DEFAULT_FILE_NAME);
}

void TabView::saveAllTabs() {
  for (int i = 0; i < count(); i++) {
    auto editView = qobject_cast<TextEditView*>(widget(i));
    if (editView) {
      editView->save();
    }
  }
}

void TabView::closeActiveTab() { closeTab(currentWidget()); }

bool TabView::closeAllTabs() {
  std::list<QWidget*> widgets;
  for (int i = 0; i < count(); i++) {
    widgets.push_back(widget(i));
  }

  for (auto w : widgets) {
    bool isSuccess = closeTab(w);
    if (!isSuccess)
      return false;
  }

  return true;
}

void TabView::closeOtherTabs() {
  std::list<QWidget*> widgets;
  for (int i = 0; i < count(); i++) {
    if (i != currentIndex()) {
      widgets.push_back(widget(i));
    }
  }

  for (auto w : widgets) {
    closeTab(w);
  }
}

int TabView::indexOfPath(const QString& path) {
  //  qDebug("TabView::indexOfPath(%s)", qPrintable(path));
  for (int i = 0; i < count(); i++) {
    TextEditView* v = qobject_cast<TextEditView*>(widget(i));
    QString path2 = v->path();
    if (v && !path2.isEmpty() && path == path2) {
      return i;
    }
  }

  return -1;
}

void TabView::detachTabStarted(int index, const QPoint&) {
  qDebug("DetachTabStarted");
  m_tabDragging = true;
  DraggingTabInfo::setWidget(widget(index));
  DraggingTabInfo::setTabText(tabText(index));
  removeTab(index);
  Q_ASSERT(DraggingTabInfo::widget());
}

void TabView::detachTabEntered(const QPoint& enterPoint) {
  qDebug("DetachTabEntered");
  qDebug() << "tabBar()->mapToGlobal(QPoint(0, 0)):" << tabBar()->mapToGlobal(QPoint(0, 0));
  QPoint relativeEnterPos = enterPoint - tabBar()->mapToGlobal(QPoint(0, 0));
  int index = tabBar()->tabAt(relativeEnterPos);
  int newIndex = insertTab(index, DraggingTabInfo::widget(), DraggingTabInfo::tabText());
  DraggingTabInfo::setWidget(nullptr);
  m_tabDragging = false;
  tabRemoved(-1);
  QPoint tabCenterPos = tabBar()->tabRect(newIndex).center();

  qDebug() << "tabCenterPos:" << tabCenterPos << "enterPoint:" << enterPoint
           << "relativeEnterPos:" << relativeEnterPos;
  m_tabBar->startMovingTab(tabCenterPos);
}

void TabView::tabInserted(int index) {
  setCurrentIndex(index);
  QTabWidget::tabInserted(index);
}

void TabView::tabRemoved(int) {
  if (count() == 0) {
    emit allTabRemoved();
  }
}

void TabView::mouseReleaseEvent(QMouseEvent* event) {
  qDebug("mouseReleaseEvent in TabView");
  QTabWidget::mouseReleaseEvent(event);
}

void TabView::setActiveEditView(TextEditView* editView) {
  if (m_activeEditView != editView) {
    TextEditView* oldEditView = m_activeEditView;
    m_activeEditView = editView;
    emit activeTextEditViewChanged(oldEditView, editView);
  }
}

void TabView::removeTabAndWidget(int index) {
  if (auto w = widget(index)) {
    w->deleteLater();
  }
  removeTab(index);
}

bool TabView::closeTab(QWidget* w) {
  TextEditView* editView = qobject_cast<TextEditView*>(w);
  if (editView && editView->document()->isModified()) {
    QMessageBox msgBox;
    msgBox.setText(tr("Do you want to save the changes made to the document %1?")
                       .arg(getFileNameFrom(editView->path())));
    msgBox.setInformativeText(tr("Your changes will be lost if you don’t save them."));
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setIconPixmap(QIcon(":/app_icon_064.png").pixmap(64, 64));
    int ret = msgBox.exec();
    switch (ret) {
      case QMessageBox::Save:
        editView->save();
        break;
      case QMessageBox::Discard:
        break;
      case QMessageBox::Cancel:
        return false;
      default:
        qWarning("ret is invalid");
        return false;
    }
  } else {
    qDebug("widget is not TextEditView or not modified");
  }

  removeTabAndWidget(indexOf(w));
  return true;
}

void TabView::updateTabTextBasedOn(bool changed) {
  qDebug() << "updateTabTextBasedOn. changed:" << changed;
  if (QWidget* w = qobject_cast<QWidget*>(QObject::sender())) {
    int index = indexOf(w);
    QString text = tabText(index);
    if (changed) {
      setTabText(index, text + "*");
    } else if (text.endsWith('*')) {
      text.chop(1);
      setTabText(index, text);
    }
  } else {
    qDebug("sender is null or not QWidget");
  }
}

void TabView::detachTabFinished(const QPoint& dropPoint) {
  qDebug() << "DetachTab."
           << "dropPoint:" << dropPoint;
  MainWindow* window = MainWindow::create();
  window->move(dropPoint);
  window->show();
  if (DraggingTabInfo::widget()) {
    window->activeTabView()->addTab(DraggingTabInfo::widget(), DraggingTabInfo::tabText());
    DraggingTabInfo::setWidget(nullptr);
    m_tabDragging = false;
    tabRemoved(-1);
  } else {
    qWarning("draggign widget is null");
  }
}
