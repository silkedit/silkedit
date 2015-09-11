#pragma once

#include <QAbstractListModel>

#include "core/macros.h"

namespace core {

class HistoryModel : public QAbstractListModel {
  Q_OBJECT
  DISABLE_COPY(HistoryModel)

 public:
  explicit HistoryModel(QObject* parent = nullptr) : QAbstractListModel(parent) {}
  ~HistoryModel() = default;
  DEFAULT_MOVE(HistoryModel)

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
  bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

  void prepend(const QString& str);

 private:
  QStringList m_stringList;
};

}  // namespace core
