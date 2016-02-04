#pragma once

#include <QTreeView>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>

#include "core/macros.h"

class MyFileSystemModel;

class ProjectTreeView : public QTreeView {
  Q_OBJECT
  DISABLE_COPY(ProjectTreeView)

 public:
  ProjectTreeView(QWidget* parent = nullptr);
  ~ProjectTreeView();
  DEFAULT_MOVE(ProjectTreeView)

  bool open(const QString& dirName);
  void edit(const QModelIndex& index);

 protected:
  void contextMenuEvent(QContextMenuEvent* event) override;
  bool edit(const QModelIndex& index, EditTrigger trigger, QEvent* event) override;

 private:
  MyFileSystemModel* m_model;
  QMetaObject::Connection m_connection;

  void createNewFile(const QDir& dir);
  void createNewDir(const QDir& dir);

 private slots:
  void open(QModelIndex index);
  void rename();
  void remove();
  void showInFinder();
  void createNewFile();
  void createNewDir();
  void focusRootDirectory(const QString& path);
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
  virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

 private:
  QString dir;
};
