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
      m_copy(new QAction(tr("Copy"), this)) {
  setModel(m_model);
  horizontalHeader()->setStretchLastSection(true);
  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  connect(m_copy, &QAction::triggered, this, [=] {
    if (const auto& keymap = m_model->keymapAt(currentIndex().row())) {
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

void KeymapTableView::contextMenuEvent(QContextMenuEvent* event) {
  QMenu menu(this);
  menu.addAction(m_copy);
  menu.exec(event->globalPos());
}

KeymapTableModel::KeymapTableModel(QObject* parent) : QAbstractTableModel(parent) {
  for (const auto& it : KeymapManager::singleton().keymaps()) {
    m_keymaps.append(Keymap{it.first, it.second});
  }
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
    return int(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if (role == Qt::DisplayRole) {
    const Keymap& keymap = m_keymaps[index.row()];
    switch (index.column()) {
      case 0:
        return QString(keymap.cmd.cmdName());
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
    case 0:
      return tr("Command");
    case 1:
      return tr("Description");
    case 2:
      return tr("Key");
    case 3:
      return tr("If");
    case 4:
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
