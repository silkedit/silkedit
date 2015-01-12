#pragma once

#include <QTreeView>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>

#include "macros.h"

class ProjectTreeView : public QTreeView {
  Q_OBJECT
  DISABLE_COPY(ProjectTreeView)

 public:
  ProjectTreeView(QWidget* parent = nullptr);
  ~ProjectTreeView();
  DEFAULT_MOVE(ProjectTreeView)

  bool open(const QString& dirName);

 private slots:
  void open(QModelIndex index);
};

/**
*@brief Show only file system hierarchy
*
* QFileSystemModel provides not only file system hierarchy but other info like size, type, etc.
*
*/
class MyFileSystemModel : public QFileSystemModel {
  Q_OBJECT
  DISABLE_COPY(MyFileSystemModel)

 public:
  MyFileSystemModel(QObject* parent);
  ~MyFileSystemModel() = default;
  DEFAULT_MOVE(MyFileSystemModel)

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};

class FilterModel : public QSortFilterProxyModel {
  Q_OBJECT

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
