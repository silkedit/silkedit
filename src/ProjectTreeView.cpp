#include <functional>
#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QContextMenuEvent>
#include <QDir>

#include "ProjectTreeView.h"
#include "DocumentManager.h"
#include "PlatformUtil.h"

namespace {

QString getUniqueName(const QString& baseNewFilePath, std::function<bool(const QString&)> pred) {
  QString newFilePath = baseNewFilePath;
  int i = 2;
  while (pred(newFilePath)) {
    newFilePath = baseNewFilePath + QString::number(i);
    i++;
  }
  return newFilePath;
}

QString getUniqueFileName(const QString& baseNewFilePath) {
  return getUniqueName(baseNewFilePath,
                       [](const QString& newFilePath) { return QFile(newFilePath).exists(); });
}

QString getUniqueDirName(const QString& baseNewDirPath) {
  return getUniqueName(baseNewDirPath,
                       [](const QString& newDirPath) { return QDir(newDirPath).exists(); });
}
}

ProjectTreeView::ProjectTreeView(QWidget* parent) : QTreeView(parent), m_model(nullptr) {
  setHeaderHidden(true);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setAttribute(Qt::WA_MacShowFocusRect, false);
  setFocusPolicy(Qt::ClickFocus);
  connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(open(QModelIndex)));
}

bool ProjectTreeView::open(const QString& dirPath) {
  QDir targetDir(dirPath);
  if (targetDir.exists()) {
    if (m_model) {
      m_model->deleteLater();
      m_model = nullptr;
    }

    m_model = new MyFileSystemModel(this);
    m_connection =
        connect(m_model, &QFileSystemModel::directoryLoaded, this, [=](const QString& path) {
          qDebug() << "directoryLoaded" << path;
          QObject::disconnect(m_connection);
          focusRootDirectory(path);
        });

    FilterModel* const filter = new FilterModel(this, dirPath);
    filter->setSourceModel(m_model);

    setModel(filter);
    if (targetDir.isRoot()) {
      // To show all drives on Windows, set "" as root path
      m_model->setRootPath("");
      setRootIndex(filter->mapFromSource(m_model->index(m_model->rootPath())));
    } else {
      m_model->setRootPath(dirPath);
      QDir parentDir(dirPath);
      parentDir.cdUp();
      QModelIndex rootIndex = filter->mapFromSource(m_model->index(parentDir.absolutePath()));
      setRootIndex(rootIndex);
    }

    expandAll();
    return true;
  } else {
    qWarning("%s doesn't exist", qPrintable(dirPath));
    return false;
  }
}

void ProjectTreeView::edit(const QModelIndex& index) {
  QTreeView::edit(index);
}

void ProjectTreeView::contextMenuEvent(QContextMenuEvent* event) {
  QMenu menu(this);
  menu.addAction(tr("Rename"), this, SLOT(rename()));
  menu.addAction(tr("Delete"), this, SLOT(remove()));
  menu.addAction(tr("New File"), this, SLOT(createNewFile()));
  menu.addAction(tr("New Folder"), this, SLOT(createNewDir()));
  menu.addAction(PlatformUtil::showInFinderText(), this, SLOT(showInFinder()));
  menu.exec(event->globalPos());
}

bool ProjectTreeView::edit(const QModelIndex& index,
                           QAbstractItemView::EditTrigger trigger,
                           QEvent* event) {
  // disable renaming by double click
  if (trigger == QAbstractItemView::DoubleClicked)
    return false;
  return QTreeView::edit(index, trigger, event);
}

void ProjectTreeView::open(QModelIndex index) {
  if (!index.isValid()) {
    qWarning("index is invalid");
    return;
  }

  FilterModel* filter = qobject_cast<FilterModel*>(model());
  if (filter && m_model) {
    QString filePath = m_model->filePath(filter->mapToSource(index));
    DocumentManager::singleton().open(filePath);
  }
}

void ProjectTreeView::rename() {
  QTreeView::edit(currentIndex());
}

void ProjectTreeView::remove() {
  QModelIndexList indices = selectedIndexes();
  foreach (const QModelIndex& filterIndex, indices) {
    FilterModel* filter = qobject_cast<FilterModel*>(model());
    if (filter && m_model) {
      QModelIndex index = filter->mapToSource(filterIndex);
      QString filePath = m_model->filePath(index);
      QFileInfo info(filePath);
      if (info.isFile()) {
        if (!m_model->remove(index)) {
          qDebug("failed to remove %s", qPrintable(filePath));
        }
      } else if (info.isDir()) {
        if (!QDir(filePath).removeRecursively()) {
          qDebug("failed to remove %s", qPrintable(filePath));
        }
      } else {
        qWarning("%s is neither file nor directory", qPrintable(filePath));
      }
    }
  }
}

void ProjectTreeView::showInFinder() {
  FilterModel* filter = qobject_cast<FilterModel*>(model());
  if (filter && m_model) {
    QString filePath = m_model->filePath(filter->mapToSource(currentIndex()));
    PlatformUtil::showInFinder(filePath);
  }
}

void ProjectTreeView::createNewFile() {
  FilterModel* filter = qobject_cast<FilterModel*>(model());
  if (filter && m_model) {
    QString filePath = m_model->filePath(filter->mapToSource(currentIndex()));
    QFileInfo info(filePath);
    if (info.isDir()) {
      createNewFile(QDir(filePath));
    } else if (info.isFile()) {
      createNewFile(info.dir());
    } else {
      qWarning("%s is neither file nor directory", qPrintable(filePath));
    }
  }
}

void ProjectTreeView::createNewDir() {
  FilterModel* filter = qobject_cast<FilterModel*>(model());
  if (filter && m_model) {
    QString filePath = m_model->filePath(filter->mapToSource(currentIndex()));
    QFileInfo info(filePath);
    if (info.isDir()) {
      createNewDir(QDir(filePath));
    } else if (info.isFile()) {
      createNewDir(info.dir());
    } else {
      qWarning("%s is neither file nor directory", qPrintable(filePath));
    }
  }
}

void ProjectTreeView::focusRootDirectory(const QString& path) {
  FilterModel* filter = qobject_cast<FilterModel*>(model());
  // it causes crash somehow...
  // This seems to be related to these issues which seem to be related with QAccessibleTableCell
  // https://bugreports.qt.io/browse/QTBUG-43796
  // https://bugreports.qt.io/browse/QTBUG-49907
  //  setFocus();
  selectionModel()->select(filter->mapFromSource(m_model->index(path)),
                           QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

void ProjectTreeView::createNewFile(const QDir& dir) {
  FilterModel* filter = qobject_cast<FilterModel*>(model());
  QFile newFile(getUniqueFileName(dir.absoluteFilePath("untitled")));
  if (!newFile.open(QIODevice::WriteOnly))
    return;
  newFile.close();
  expand(currentIndex());
  QModelIndex index = filter->mapFromSource(m_model->index(newFile.fileName()));
  edit(index);
}

void ProjectTreeView::createNewDir(const QDir& dir) {
  FilterModel* filter = qobject_cast<FilterModel*>(model());
  QDir newDir(getUniqueDirName(dir.absoluteFilePath("untitled folder")));
  if (dir.mkdir(newDir.dirName())) {
    expand(currentIndex());
    QModelIndex index = filter->mapFromSource(m_model->index(newDir.absolutePath()));
    edit(index);
  }
}

MyFileSystemModel::MyFileSystemModel(QObject* parent) : QFileSystemModel(parent) {
  setReadOnly(false);
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

// Show only specific directory content
bool FilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
  QModelIndex pathIndex = sourceModel()->index(sourceRow, 0, sourceParent);
  // Without trailing /, can't distinguish hoge from hoge2
  QString path = sourceModel()->data(pathIndex, QFileSystemModel::FilePathRole).toString() + "/";
  return dir.startsWith(path, Qt::CaseInsensitive) || path.startsWith(dir, Qt::CaseInsensitive);
}
