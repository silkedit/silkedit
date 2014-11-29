#include "STabWidget.h"

#include "TextEditView.h"

STabWidget::STabWidget(QWidget* parent) : QTabWidget(parent) {
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

int STabWidget::addTextEditView(TextEditView* view, const QString& label) {
  for (int i = 0; i < count(); i++) {
    TextEditView* v = qobject_cast<TextEditView*>(widget(i));
    boost::optional<QString> path1 = view->path();
    boost::optional<QString> path2 = v->path();
    if (v && path1 && path2 && *path1 == *path2) {
      setCurrentIndex(i);
      return 0;
    }
  }

  return addTab(view, label);
}

void STabWidget::tabInserted(int index) {
  setCurrentIndex(index);
  QTabWidget::tabInserted(index);
}
