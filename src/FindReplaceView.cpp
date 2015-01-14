#include <QVBoxLayout>
#include <QLineEdit>

#include "FindReplaceView.h"

FindReplaceView::FindReplaceView(QWidget* parent) : QWidget(parent) {
  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(new QLineEdit);
  layout->addWidget(new QLineEdit);
  setLayout(layout);
}
