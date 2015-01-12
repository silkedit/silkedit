#include <QApplication>
#include <QDebug>
#include <QSortFilterProxyModel>

#include "ProjectTreeView.h"

class FilterModel : public QSortFilterProxyModel {
 public:
  FilterModel(QObject* parent, const QString& targetDir)
      : QSortFilterProxyModel(parent), dir(targetDir) {
    if (!dir.endsWith("/")) {
      dir += "/";
    }
  }

 protected:
  virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
    QString path;
    QModelIndex pathIndex = source_parent.child(source_row, 0);
    while (pathIndex.parent().isValid()) {
      path = sourceModel()->data(pathIndex).toString() + "/" + path;
      pathIndex = pathIndex.parent();
    }
    // Get the leading "/" on Linux. Drive on Windows?
    path = sourceModel()->data(pathIndex).toString() + path;

    // First test matches paths before we've reached the target directory.
    // Second test matches paths after we've passed the target directory.
    return dir.startsWith(path) || path.startsWith(dir);
  }

 private:
  QString dir;
};

ProjectTreeView::ProjectTreeView(QWidget* parent) : QTreeView(parent) {
  setHeaderHidden(true);
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
