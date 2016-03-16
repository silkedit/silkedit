#pragma once

#include <QTreeView>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>

#include "core/macros.h"

class MyFileSystemModel;
namespace core {
    class Theme;
}

class ProjectTreeView : public QTreeView {
  Q_OBJECT
  DISABLE_COPY(ProjectTreeView)

 public:
  ProjectTreeView(QWidget* parent = nullptr);
  ~ProjectTreeView();
  DEFAULT_MOVE(ProjectTreeView)

  bool openDirOrExpand(const QString& dirName);
  void edit(const QModelIndex& index);
  QString dirPath() const;

 protected:
  void contextMenuEvent(QContextMenuEvent* event) override;
  bool edit(const QModelIndex& index, EditTrigger trigger, QEvent* event) override;
  void keyPressEvent(QKeyEvent * event) override;
  void mouseDoubleClickEvent(QMouseEvent *) override;

 private:
  MyFileSystemModel* m_model;
  QMetaObject::Connection m_connection;

  void setTheme(const core::Theme* theme);
  void setFont(const QFont& font);
  void createNewFile(const QDir& dir);
  void createNewDir(const QDir& dir);

 private:
  void openOrExpand(QModelIndex index);
  void focusRootDirectory(const QString& path);

private slots:
  void rename();
  void remove();
  void showInFinder();
  void createNewFile();
  void createNewDir();
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
    if (!dir.endsWith(QStringLiteral("/"))) {
      dir += QStringLiteral("/");
    }
  }

  QString dirPath() { return dir; }

 protected:
  virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

 private:
  QString dir;
};
