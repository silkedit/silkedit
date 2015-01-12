#include <QApplication>
#include <QDebug>

#include "ProjectTreeView.h"

ProjectTreeView::ProjectTreeView(QWidget* parent) : QTreeView(parent) {
  setHeaderHidden(true);
  MyFileSystemModel* model = new MyFileSystemModel;
  model->removeColumns(1, 3);
  model->setRootPath(QDir::currentPath());
  setModel(model);
}

void ProjectTreeView::mousePressEvent(QMouseEvent* event) {
  if (QApplication::focusWidget() == this) {
    clearFocus();
  }
  QTreeView::mousePressEvent(event);
}

void ProjectTreeView::mouseDoubleClickEvent(QMouseEvent* event) {
  if (QApplication::focusWidget() == this) {
    clearFocus();
  }
  QTreeView::mouseDoubleClickEvent(event);
}

int MyFileSystemModel::columnCount(const QModelIndex&) const {
  return 1;
}

QVariant MyFileSystemModel::data(const QModelIndex& index, int role) const {
  if (index.column() == 0) {
    return QFileSystemModel::data(index, role);
  } else {
    return QVariant();
  }
}
