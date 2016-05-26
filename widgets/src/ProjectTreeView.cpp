#include <functional>
#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QContextMenuEvent>
#include <QDir>
#include <QScrollBar>
#include <QHeaderView>

#include "ProjectTreeView.h"
#include "DocumentManager.h"
#include "PlatformUtil.h"
#include "core/Config.h"
#include "core/Theme.h"
#include "core/Util.h"
#include "core/Constants.h"

using core::Util;
using core::Config;
using core::Theme;
using core::ColorSettings;
using core::Constants;

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
  setTheme(Config::singleton().theme());
  setFont();

  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setAttribute(Qt::WA_MacShowFocusRect, false);
  setFocusPolicy(Qt::ClickFocus);

  setHeaderHidden(true);
  // We let the column adjust to contents, but note the setting of a minimum size in resizeEvent()
  header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  header()->setStretchLastSection(false);

  connect(this, &ProjectTreeView::activated, this, &ProjectTreeView::openOrExpand);
  connect(&Config::singleton(), &Config::themeChanged, this, &ProjectTreeView::setTheme);
  connect(&Config::singleton(), &Config::fontChanged, this, &ProjectTreeView::setFont);
}

bool ProjectTreeView::openDirOrExpand(const QString& dirPath) {
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

QString ProjectTreeView::dirPath() const {
  if (auto filterModel = qobject_cast<FilterModel*>(model())) {
    return filterModel->dirPath();
  }

  return QString();
}

void ProjectTreeView::setTheme(const core::Theme* theme) {
  qDebug("ProjectTreeView theme is changed");
  if (!theme) {
    qWarning("theme is null");
    return;
  }

  if (theme->projectTreeViewSettings) {
    QString style =
        QStringLiteral(
            R"(
ProjectTreeView {
  background-color: %1;
  color: %2;
  selection-background-color: %3
}

ProjectTreeView::item:selected {
  background-color: %3;
  color:%4;
}
)")
            .arg(Util::qcolorForStyleSheet(
                     theme->projectTreeViewSettings->value(QStringLiteral("background"))),
                 Util::qcolorForStyleSheet(
                     theme->projectTreeViewSettings->value(QStringLiteral("foreground"))),
                 Util::qcolorForStyleSheet(
                     theme->projectTreeViewSettings->value(QStringLiteral("selectionBackground"))),
                 Util::qcolorForStyleSheet(
                     theme->projectTreeViewSettings->value(QStringLiteral("selectionForeground"))));

// The branch indicator icon is hidden in a dark theme on Windows, so we use a custom icon
#ifdef Q_OS_WIN
    QString branchClosedImage;
    QString branchOpenImage;
    int padding = -1;

    if (theme->isDarkTheme()) {
      branchClosedImage = ":/images/branch-closed-white.svg";
      branchOpenImage = ":/images/branch-open-white.svg";
      padding = 5;
    } else {
      branchClosedImage = ":/images/branch-closed-black.svg";
      branchOpenImage = ":/images/branch-open-black.svg";
      padding = 6;
    }

    Q_ASSERT(!branchClosedImage.isEmpty());
    Q_ASSERT(!branchOpenImage.isEmpty());
    Q_ASSERT(padding >= 0);

      style += QStringLiteral(R"(
ProjectTreeView::branch:has-children:!has-siblings:closed,
ProjectTreeView::branch:closed:has-children:has-siblings {
  border-image: none;
  image: url(%1);
  padding: %2px;
}

ProjectTreeView::branch:open:has-children:!has-siblings,
ProjectTreeView::branch:open:has-children:has-siblings  {
  border-image: none;
  image: url(%3);
  padding: %2px;
}
                            )").arg(branchClosedImage).arg(padding).arg(branchOpenImage);
#endif

    this->setStyleSheet(style);

    verticalScrollBar()->setStyleSheet(theme->projectTreeViewVerticalScrollBarStyle());
    horizontalScrollBar()->setStyleSheet(theme->projectTreeViewHorizontalScrollBarStyle());
  }
}

void ProjectTreeView::setFont() {
  QFont projectTreeFont(Constants::defaultUIFontFamily);
  projectTreeFont.setPointSize(Constants::defaultUIFontSize);
  QTreeView::setFont(projectTreeFont);
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

void ProjectTreeView::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Return && state() != State::EditingState) {
    emit activated(currentIndex());
  } else {
    QTreeView::keyPressEvent(event);
  }
}

void ProjectTreeView::mouseDoubleClickEvent(QMouseEvent*) {
  emit activated(currentIndex());
}

void ProjectTreeView::resizeEvent(QResizeEvent* event) {
  header()->setMinimumSectionSize(viewport()->width());
  QTreeView::resizeEvent(event);
}

void ProjectTreeView::openOrExpand(QModelIndex index) {
  if (!index.isValid()) {
    qWarning("index is invalid");
    return;
  }

  FilterModel* filter = qobject_cast<FilterModel*>(model());
  if (filter && m_model) {
    QString path = m_model->filePath(filter->mapToSource(index));
    if (QFileInfo(path).isFile()) {
      DocumentManager::singleton().open(path);
    } else {
      setExpanded(index, !isExpanded(index));
    }
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
  QFile newFile(getUniqueFileName(dir.absoluteFilePath(QStringLiteral("untitled"))));
  if (!newFile.open(QIODevice::WriteOnly))
    return;
  newFile.close();
  expand(currentIndex());
  QModelIndex index = filter->mapFromSource(m_model->index(newFile.fileName()));
  edit(index);
}

void ProjectTreeView::createNewDir(const QDir& dir) {
  FilterModel* filter = qobject_cast<FilterModel*>(model());
  QDir newDir(getUniqueDirName(dir.absoluteFilePath(QStringLiteral("untitled folder"))));
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
  QString path = sourceModel()->data(pathIndex, QFileSystemModel::FilePathRole).toString() +
                 QStringLiteral("/");
  return dir.startsWith(path, Qt::CaseInsensitive) || path.startsWith(dir, Qt::CaseInsensitive);
}
