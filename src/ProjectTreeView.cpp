#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QContextMenuEvent>

#include "ProjectTreeView.h"
#include "DocumentService.h"

ProjectTreeView::ProjectTreeView(QWidget* parent) : QTreeView(parent) {
  setHeaderHidden(true);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setAttribute(Qt::WA_MacShowFocusRect, false);
  connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(open(QModelIndex)));
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

void ProjectTreeView::contextMenuEvent(QContextMenuEvent* event) {
  QMenu menu(this);
  menu.addAction(tr("Rename"), this, SLOT(rename()));
  menu.exec(event->globalPos());
}

void ProjectTreeView::mouseDoubleClickEvent(QMouseEvent *)
{
  emit activated(currentIndex());
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

void ProjectTreeView::rename() {
  edit(currentIndex());
}

MyFileSystemModel::MyFileSystemModel(QObject* parent) : QFileSystemModel(parent) {
  setReadOnly(false);
  removeColumns(1, 3);
}

int MyFileSystemModel::columnCount(const QModelIndex&) const { return 1; }

QVariant MyFileSystemModel::data(const QModelIndex& index, int role) const {
  if (index.column() == 0) {
    return QFileSystemModel::data(index, role);
  } else {
    return QVariant();
  }
}

ProjectTreeView::~ProjectTreeView() { qDebug("~ProjectTreeView"); }
