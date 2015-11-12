#include <QApplication>
#include <QClipboard>
#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>
#include <QHeaderView>

#include "KeymapTableView.h"
#include "KeymapManager.h"
#include "core/Util.h"

using core::Util;

KeymapTableView::KeymapTableView(QWidget* parent)
    : QTableView(parent),
      m_model(new KeymapTableModel(this)),
      m_copy(new QAction(tr("Copy"), this)),
      m_proxyModel(new KeymapSortFilterProxyModel(m_model, this)) {
  setModel(m_proxyModel);
  horizontalHeader()->setSortIndicator(KeymapTableModel::SOURCE_INDEX, Qt::AscendingOrder);
  horizontalHeader()->setStretchLastSection(true);
  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  connect(m_copy, &QAction::triggered, this, [=] {
    if (const auto& keymap = m_model->keymapAt(m_proxyModel->mapToSource(currentIndex()).row())) {
      QClipboard* clipboard = QApplication::clipboard();
      assert(clipboard);
      if (keymap->cmd.condition()) {
        clipboard->setText(QString("- { key: %1, command: %2, if: %3 }")
                               .arg(Util::toString(keymap->key))
                               .arg(keymap->cmd.cmdName())
                               .arg(keymap->cmd.condition()->toString()));
      } else {
        clipboard->setText(QString("- { key: %1, command: %2 }")
                               .arg(Util::toString(keymap->key))
                               .arg(keymap->cmd.cmdName()));
      }
    }
  });
}

void KeymapTableView::setFilterText(const QString& text) {
  m_proxyModel->setFilterText(text);
}

void KeymapTableView::contextMenuEvent(QContextMenuEvent* event) {
  QMenu menu(this);
  menu.addAction(m_copy);
  menu.exec(event->globalPos());
}

void KeymapTableModel::init() {
  for (const auto& it : KeymapManager::singleton().keymaps()) {
    m_keymaps.append(Keymap{it.first, it.second});
  }
}

KeymapTableModel::KeymapTableModel(QObject* parent) : QAbstractTableModel(parent) {
  init();
  connect(&KeymapManager::singleton(), &KeymapManager::keymapUpdated, this, [=] {
    m_keymaps.clear();
    init();
  });
}

int KeymapTableModel::rowCount(const QModelIndex&) const {
  return m_keymaps.size();
}

int KeymapTableModel::columnCount(const QModelIndex&) const {
  return 5;
}

QVariant KeymapTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() >= m_keymaps.size()) {
    return QVariant();
  }

  if (role == Qt::TextAlignmentRole) {
    return int(Qt::AlignVCenter);
  } else if (role == Qt::DisplayRole) {
    const Keymap& keymap = m_keymaps[index.row()];
    switch (index.column()) {
      case 0:
        return keymap.cmd.cmdName();
      case 1:
        return keymap.cmd.cmdDescription();
      case 2:
        return keymap.key.toString(QKeySequence::NativeText);
      case 3:
        return keymap.cmd.condition() ? keymap.cmd.condition()->toString() : QVariant();
      case 4:
        return keymap.cmd.source();
      default:
        return QVariant();
    }
  }
  return QVariant();
}

QVariant KeymapTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Vertical) {
    return QVariant();
  }

  switch (section) {
    case COMMAND_INDEX:
      return tr("Command");
    case DESCRIPTION_INDEX:
      return tr("Description");
    case KEY_INDEX:
      return tr("Key");
    case IF_INDEX:
      return tr("If");
    case SOURCE_INDEX:
      return tr("Source");
    default:
      return QVariant();
  }
}

boost::optional<Keymap> KeymapTableModel::keymapAt(int row) {
  if (row < m_keymaps.size()) {
    return m_keymaps.at(row);
  }
  return boost::none;
}

KeymapSortFilterProxyModel::KeymapSortFilterProxyModel(KeymapTableModel* model, QObject* parent)
    : QSortFilterProxyModel(parent) {
  setSourceModel(model);
}

void KeymapSortFilterProxyModel::setFilterText(const QString& text) {
  m_filterText = text;
  invalidateFilter();
}

bool KeymapSortFilterProxyModel::filterAcceptsRow(int sourceRow,
                                                  const QModelIndex& sourceParent) const {
  if (m_filterText.isEmpty())
    return true;

  QModelIndex cmdIndex =
      sourceModel()->index(sourceRow, KeymapTableModel::COMMAND_INDEX, sourceParent);
  QModelIndex descIndex =
      sourceModel()->index(sourceRow, KeymapTableModel::DESCRIPTION_INDEX, sourceParent);
  QModelIndex keyIndex = sourceModel()->index(sourceRow, KeymapTableModel::KEY_INDEX, sourceParent);
  QModelIndex ifIndex = sourceModel()->index(sourceRow, KeymapTableModel::IF_INDEX, sourceParent);
  QModelIndex sourceIndex =
      sourceModel()->index(sourceRow, KeymapTableModel::SOURCE_INDEX, sourceParent);
  QString keyText = Util::toString(QKeySequence(sourceModel()->data(keyIndex).toString()));

  return sourceModel()->data(cmdIndex).toString().contains(m_filterText, Qt::CaseInsensitive) ||
         sourceModel()->data(descIndex).toString().contains(m_filterText, Qt::CaseInsensitive) ||
         (keyText.contains(m_filterText, Qt::CaseInsensitive)) ||
         sourceModel()->data(ifIndex).toString().contains(m_filterText, Qt::CaseInsensitive) ||
         sourceModel()->data(sourceIndex).toString().contains(m_filterText, Qt::CaseInsensitive);
}
