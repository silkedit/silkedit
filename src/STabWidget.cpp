#include "STabWidget.h"

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

STabWidget::~STabWidget()
{
  qDebug("~STabWidget");
}

int STabWidget::addTab(QWidget* widget, const QString& label) {
  int index = QTabWidget::addTab(widget, label);
  m_widgets.insert(set_unique_ptr<QWidget>(widget));
  return index;
}

void STabWidget::tabInserted(int index) {
  setCurrentIndex(index);
  QTabWidget::tabInserted(index);
}
