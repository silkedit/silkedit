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

STabWidget::STabWidget(QWidget* parent) : QTabWidget(parent), m_tabBar(new STabBar(this)) {
  connect(
      m_tabBar, SIGNAL(OnDetachTab(int, const QPoint&)), this, SLOT(DetachTab(int, const QPoint&)));

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
  disconnect(
      m_tabBar, SIGNAL(OnDetachTab(int, const QPoint&)), this, SLOT(DetachTab(int, const QPoint&)));
}

int STabWidget::addTab(QWidget* page, const QString& label) {
  page->setParent(this);
  int index = QTabWidget::addTab(page, label);
  return index;
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

void STabWidget::tabInserted(int index) {
  setCurrentIndex(index);
  QTabWidget::tabInserted(index);
}

void STabWidget::tabRemoved(int) {
  if (count() == 0) {
    emit allTabRemoved();
  }
}

void STabWidget::DetachTab(int index, const QPoint& dropPoint) {
  qDebug() << "DetachTab."
           << "dropPoint: " << dropPoint;
  MainWindow* window = MainWindow::create();
  window->show();
  window->move(dropPoint);
  QWidget* w = widget(index);
  if (w) {
    window->tabBar()->addTab(w, tabText(index));
  }
}
