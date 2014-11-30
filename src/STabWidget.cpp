#include <memory>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QTextDocument>

#include "STabWidget.h"
#include "TextEditView.h"
#include "KeymapService.h"

STabWidget::STabWidget(QWidget* parent) : QTabWidget(parent) {
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
      int numDeleted = m_widgets.erase(make_find_ptr(w));
      qDebug("m_widgets size: %lu", m_widgets.size());
      Q_ASSERT(numDeleted == 1);
    }
  });
}

STabWidget::~STabWidget() {
  qDebug("~STabWidget");
}

int STabWidget::addTab(QWidget* page, const QString& label) {
  int index = QTabWidget::addTab(page, label);
  m_widgets.insert(set_unique_ptr<QWidget>(page));
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
