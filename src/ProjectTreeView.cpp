#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QContextMenuEvent>

#include "ProjectTreeView.h"
#include "DocumentService.h"
#include "PlatformUtil.h"

ProjectTreeView::ProjectTreeView(QWidget* parent) : QTreeView(parent), m_model(nullptr) {
  setHeaderHidden(true);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setAttribute(Qt::WA_MacShowFocusRect, false);
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
    m_model->setRootPath(dirPath);

    FilterModel* const filter = new FilterModel(this, dirPath);
    filter->setSourceModel(m_model);

    setModel(filter);
    if (targetDir.isRoot()) {
      setRootIndex(m_model->index(dirPath));
    } else {
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
    DocumentService::open(filePath);
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

void ProjectTreeView::createNewFile(const QDir& dir) {
  FilterModel* filter = qobject_cast<FilterModel*>(model());
  QString baseNewFilePath = dir.absoluteFilePath("untitled");
  QString newFilePath = baseNewFilePath;
  int i = 2;
  while (QFile(newFilePath).exists()) {
    newFilePath = baseNewFilePath + QString::number(i);
    i++;
  }
  QFile newFile(newFilePath);
  if (!newFile.open(QIODevice::WriteOnly))
    return;
  newFile.close();
  expand(currentIndex());
  QModelIndex index = filter->mapFromSource(m_model->index(newFile.fileName()));
  edit(index);
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
