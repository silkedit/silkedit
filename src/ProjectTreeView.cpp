#include <QApplication>
#include <QDebug>

#include "ProjectTreeView.h"
#include "DocumentService.h"


ProjectTreeView::ProjectTreeView(QWidget* parent) : QTreeView(parent) {
  setHeaderHidden(true);
  connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(open(QModelIndex)));
}

bool ProjectTreeView::open(const QString& dirPath) {
  QDir targetDir(dirPath);
  if (targetDir.exists()) {
    QAbstractItemModel* prevModel = model();
    if (prevModel) {
      prevModel->deleteLater();
    }

    MyFileSystemModel* model = new MyFileSystemModel(this);
    model->setRootPath(dirPath);

    FilterModel* const filter = new FilterModel(this, dirPath);
    filter->setSourceModel(model);

    setModel(filter);
    if (targetDir.isRoot()) {
      setRootIndex(model->index(dirPath));
    } else {
      QDir parentDir(dirPath);
      parentDir.cdUp();
      setRootIndex(filter->mapFromSource(model->index(parentDir.absolutePath())));
    }
    return true;
  } else {
    qWarning("%s doesn't exist", qPrintable(dirPath));
    return false;
  }
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

void ProjectTreeView::open(QModelIndex index) {
  if (!index.isValid()) {
    qWarning("index is invalid");
    return;
  }

  if (FilterModel* filter = qobject_cast<FilterModel*>(model())) {
    if (MyFileSystemModel* fsModel = qobject_cast<MyFileSystemModel*>(filter->sourceModel())) {
      QString filePath = fsModel->filePath(filter->mapToSource(index));
      DocumentService::open(filePath);
    }
  }
}

MyFileSystemModel::MyFileSystemModel(QObject* parent) : QFileSystemModel(parent) {
  removeColumns(1, 3);
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

ProjectTreeView::~ProjectTreeView() {
  qDebug("~ProjectTreeView");
}
