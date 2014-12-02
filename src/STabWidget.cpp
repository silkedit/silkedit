#include <memory>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QTextDocument>

#include "STabWidget.h"
#include "TextEditView.h"
#include "KeymapService.h"
#include "STabBar.h"
#include "MainWindow.h"

STabWidget::STabWidget(QWidget* parent)
    : QTabWidget(parent), m_tabBar(new STabBar(this)), m_draggingWidget(nullptr) {
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
    if (QWidget* w = widget(index)) {
      QTabWidget::removeTab(index);
      qDebug("tab widget (index %i) is deleted", index);
      w->deleteLater();
    }
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
  page->setParent(this);
  return QTabWidget::addTab(page, label);
}

int STabWidget::insertTab(int index, QWidget* w, const QString& label) {
  w->setParent(this);
  return QTabWidget::insertTab(index, w, label);
}

// todo: Move this method to tab group later
int STabWidget::open(const QString& path) {
  for (int i = 0; i < count(); i++) {
    TextEditView* v = qobject_cast<TextEditView*>(widget(i));
    boost::optional<QString> path2 = v->path();
    if (v && path2 && path == *path2) {
      setCurrentIndex(i);
      return i;
    }
  }

  QFile file(path);
  if (!file.open(QIODevice::ReadWrite))
    return -1;

  QTextStream in(&file);
  std::shared_ptr<QTextDocument> newDoc(new QTextDocument(in.readAll()));
  STextDocumentLayout* layout = new STextDocumentLayout(newDoc.get());
  newDoc->setDocumentLayout(layout);

  QFileInfo info(path);
  QString label(info.fileName());

  TextEditView* view = new TextEditView(path);
  view->setDocument(std::move(newDoc));
  view->installEventFilter(&KeyHandler::singleton());
  return addTab(view, label);
}

void STabWidget::addNew() {
  TextEditView* view = new TextEditView();
  view->installEventFilter(&KeyHandler::singleton());
  addTab(view, "untitled");
}

bool STabWidget::tabDragging() {
  return m_draggingWidget != nullptr;
}

void STabWidget::detachTabStarted(int index, const QPoint&) {
  qDebug("DetachTabStarted");
  m_draggingWidget = widget(index);
  m_tabText = tabText(index);
  removeTab(index);
  Q_ASSERT(m_draggingWidget);
}

void STabWidget::detachTabEntered(const QPoint& enterPoint) {
  qDebug("DetachTabEntered");
  int index = tabBar()->tabAt(enterPoint);
  int newIndex = insertTab(index, m_draggingWidget, m_tabText);
  m_draggingWidget = nullptr;
  setCurrentIndex(newIndex);
  QPoint tabCenterPos = tabBar()->tabRect(newIndex).center();

  qDebug() << "tabCenterPos:" << tabCenterPos << "enterPoint:" << enterPoint;
  m_tabBar->startMovingTab(tabCenterPos, enterPoint);
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

void STabWidget::detachTabFinished(const QPoint& dropPoint) {
  qDebug() << "DetachTab."
           << "dropPoint:" << dropPoint;
  MainWindow* window = MainWindow::create();
  window->show();
  window->move(dropPoint);
  if (m_draggingWidget) {
    window->tabWidget()->addTab(m_draggingWidget, m_tabText);
    m_draggingWidget = nullptr;
    tabRemoved(-1);
  } else {
    qWarning("draggign widget is null");
  }
}
