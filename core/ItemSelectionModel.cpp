#include "ItemSelectionModel.h"

core::ItemSelectionModel::ItemSelectionModel(QItemSelectionModel* model) {
  m_wrapped = QVariant::fromValue(model);
}

void core::ItemSelectionModel::select(const QModelIndex& index, int command) {
  m_wrapped.value<QItemSelectionModel*>()->select(
      index, static_cast<QItemSelectionModel::SelectionFlags>(command));
}
