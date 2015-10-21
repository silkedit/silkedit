#include "HistoryModel.h"

namespace {
static constexpr int HISTORY_LIMIT = 10;
}

namespace core {

int HistoryModel::rowCount(const QModelIndex&) const {
  return m_stringList.count();
}

QVariant HistoryModel::data(const QModelIndex& index, int role) const {
  if (index.row() < 0 || index.row() >= m_stringList.size())
    return QVariant();

  if (role == Qt::DisplayRole || role == Qt::EditRole)
    return m_stringList.at(index.row());

  return QVariant();
}

bool HistoryModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (index.row() >= 0 && index.row() < m_stringList.size() &&
      (role == Qt::EditRole || role == Qt::DisplayRole)) {
    m_stringList.replace(index.row(), value.toString());
    emit dataChanged(index, index, QVector<int>() << role);
    return true;
  }
  return false;
}

bool HistoryModel::insertRows(int row, int count, const QModelIndex& parent) {
  if (count < 1 || row < 0 || row > rowCount(parent))
    return false;

  beginInsertRows(QModelIndex(), row, row + count - 1);

  for (int r = 0; r < count; ++r)
    m_stringList.insert(row, QString());

  endInsertRows();

  while (rowCount() > HISTORY_LIMIT) {
    removeRow(rowCount() - 1, parent);
  }

  return true;
}

bool HistoryModel::removeRows(int row, int count, const QModelIndex& parent) {
  if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
    return false;

  beginRemoveRows(QModelIndex(), row, row + count - 1);

  for (int r = 0; r < count; ++r)
    m_stringList.removeAt(row);

  endRemoveRows();

  return true;
}

void HistoryModel::prepend(const QString& str) {
  if (!str.isEmpty() && !m_stringList.contains(str)) {
    insertRow(0);
    setData(index(0), str);
  }
}

}  // namespace core
