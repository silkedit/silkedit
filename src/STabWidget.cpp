#include "STabWidget.h"

STabWidget::STabWidget(QWidget* parent) : QTabWidget(parent) {
  QObject::connect(
      this, &QTabWidget::tabCloseRequested, [this](int index) { QTabWidget::removeTab(index); });
}

int STabWidget::addTab(std::unique_ptr<QWidget> widget, const QString& label) {
  int index = QTabWidget::addTab(widget.get(), label);
  m_widgets.insert(std::move(widget));
  return index;
}

void STabWidget::tabRemoved(int index) {
  if (QWidget* w = widget(index)) {
    int numDeleted = m_widgets.erase(std::unique_ptr<QWidget>(w));
    Q_ASSERT(numDeleted == 1);
  }

  QTabWidget::tabRemoved(index);
}
