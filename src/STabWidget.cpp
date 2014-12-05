#include <memory>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QTextDocument>
#include <QMessageBox>

#include "STabWidget.h"
#include "TextEditView.h"
#include "KeymapService.h"
#include "STabBar.h"
#include "MainWindow.h"
#include "DraggingTabInfo.h"
#include "SilkApp.h"

namespace {
QString getFileNameFrom(const QString& path) {
  QFileInfo info(path);
  return info.fileName();
}
}

STabWidget::STabWidget(QWidget* parent)
    : QTabWidget(parent), m_tabBar(new STabBar(this)), m_tabDragging(false) {
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

  QObject::connect(this, &QTabWidget::currentChanged, [this](int index) {
    // This lambda is called after m_tabbar is deleted when shutdown.
    if (index < 0)
      return;

    qDebug("currentChanged. index: %i, tab count: %i", index, count());
    if (auto w = widget(index)) {
      m_activeEditView = qobject_cast<TextEditView*>(w);
    } else {
      qDebug("active edit view is null");
      m_activeEditView = nullptr;
    }
  });

  QObject::connect(this, &QTabWidget::tabCloseRequested, [this](int index) {
      qDebug("tab widget (index %i) is deleted", index);
      removeTabAndWidget(index);
  });
}

STabWidget::~STabWidget() {
  qDebug("~STabWidget");
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

int STabWidget::addTab(QWidget* page, const QString& label) {
  return insertTab(-1, page, label);
}

int STabWidget::insertTab(int index, QWidget* w, const QString& label) {
  w->setParent(this);
  TextEditView* editView = qobject_cast<TextEditView*>(w);
  if (editView) {
    QObject::connect(editView, &TextEditView::pathUpdated, [this, editView](const QString& path) {
      setTabText(indexOf(editView), getFileNameFrom(path));
    });
    QObject::connect(editView, &TextEditView::saved, [editView]() {
      editView->document()->setModified(false);
    });
    connect(editView, SIGNAL(modificationChanged(bool)), this, SLOT(updateTabTextBasedOn(bool)));
  } else {
    qDebug("inserted widget is not TextEditView");
  }
  return QTabWidget::insertTab(index, w, label);
}

int STabWidget::open(const QString& path) {
  for (int i = 0; i < count(); i++) {
    TextEditView* v = qobject_cast<TextEditView*>(widget(i));
    QString path2 = v->path();
    if (v && !path2.isEmpty() && path == path2) {
      setCurrentIndex(i);
      return i;
    }
  }

  QFile file(path);
  if (!file.open(QIODevice::ReadWrite))
    return -1;

  QTextStream in(&file);
  std::shared_ptr<QTextDocument> newDoc(new QTextDocument(in.readAll()));
  newDoc->setModified(false);
  STextDocumentLayout* layout = new STextDocumentLayout(newDoc.get());
  newDoc->setDocumentLayout(layout);

  TextEditView* view = new TextEditView(path);
  view->setDocument(std::move(newDoc));
  return addTab(view, getFileNameFrom(path));
}

void STabWidget::addNew() {
  TextEditView* view = new TextEditView();
  addTab(view, "untitled");
}

void STabWidget::saveAllTabs() {
  for (int i = 0; i < count(); i++) {
    auto editView = qobject_cast<TextEditView*>(widget(i));
    if (editView && !editView->path().isEmpty()) {
      editView->save();
    }
  }
}

void STabWidget::closeActiveTab() {
  if (m_activeEditView) {
    if (m_activeEditView->document()->isModified()) {
      QMessageBox msgBox;
      msgBox.setText(tr("Do you want to save the changes made to the document %1?").arg(getFileNameFrom(m_activeEditView->path())));
      msgBox.setInformativeText(tr("Your changes will be lost if you don’t save them."));
      msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Save);
      msgBox.setIconPixmap(SilkApp::windowIcon().pixmap(64, 64));
      int ret = msgBox.exec();
      switch (ret) {
        case QMessageBox::Save:
            m_activeEditView->save();
            break;
        case QMessageBox::Discard:
            break;
        case QMessageBox::Cancel:
            return;
        default:
            qWarning("ret is invalid");
            return;
      }
    }
    removeTabAndWidget(indexOf(m_activeEditView));
  }
}

void STabWidget::detachTabStarted(int index, const QPoint&) {
  qDebug("DetachTabStarted");
  m_tabDragging = true;
  DraggingTabInfo::setWidget(widget(index));
  DraggingTabInfo::setTabText(tabText(index));
  removeTab(index);
  Q_ASSERT(DraggingTabInfo::widget());
}

void STabWidget::detachTabEntered(const QPoint& enterPoint) {
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

void STabWidget::tabInserted(int index) {
  setCurrentIndex(index);
  QTabWidget::tabInserted(index);
}

void STabWidget::tabRemoved(int) {
  if (count() == 0) {
    emit allTabRemoved();
  }
}

void STabWidget::mouseReleaseEvent(QMouseEvent* event) {
  qDebug("mouseReleaseEvent in STabWidget");
  QTabWidget::mouseReleaseEvent(event);
}

void STabWidget::removeTabAndWidget(int index)
{
  if (auto w = widget(index)) {
    w->deleteLater();
  }
  removeTab(index);
}

void STabWidget::updateTabTextBasedOn(bool changed)
{
  qDebug("modificationChanged");
  if (QWidget* w = qobject_cast<QWidget*>(QObject::sender())) {
    int index = indexOf(w);
    QString text = tabText(index);
    if (changed) {
      setTabText(index, text + "*");
    } else if(text.endsWith('*')) {
      text.chop(1);
      setTabText(index, text);
    }
  } else {
    qDebug("sender is null or not QWidget");
  }
}

void STabWidget::detachTabFinished(const QPoint& dropPoint) {
  qDebug() << "DetachTab."
           << "dropPoint:" << dropPoint;
  MainWindow* window = MainWindow::create();
  window->move(dropPoint);
  window->show();
  if (DraggingTabInfo::widget()) {
    window->activeTabWidget()->addTab(DraggingTabInfo::widget(), DraggingTabInfo::tabText());
    DraggingTabInfo::setWidget(nullptr);
    m_tabDragging = false;
    tabRemoved(-1);
  } else {
    qWarning("draggign widget is null");
  }
}
