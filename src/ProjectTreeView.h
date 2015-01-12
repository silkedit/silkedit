#pragma once

#include <QTreeView>
#include <QFileSystemModel>

#include "macros.h"

class ProjectTreeView : public QTreeView {
  Q_OBJECT
  DISABLE_COPY(ProjectTreeView)

 public:
  ProjectTreeView(QWidget* parent = nullptr);
  ~ProjectTreeView() = default;
  DEFAULT_MOVE(ProjectTreeView)

 protected:
  void mousePressEvent(QMouseEvent*) override;
  void mouseDoubleClickEvent ( QMouseEvent * event ) override;

 private:
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
  MyFileSystemModel() = default;
  ~MyFileSystemModel() = default;
  DEFAULT_MOVE(MyFileSystemModel)

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
};
