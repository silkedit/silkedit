#include "QAbstractItemViewWrap.h"

int core::QAbstractItemViewWrap::sizeHintForRow(int row) const {
  return m_wrapped.value<QAbstractItemView*>()->sizeHintForRow(row);
}

int core::QAbstractItemViewWrap::sizeHintForColumn(int column) const {
  return m_wrapped.value<QAbstractItemView*>()->sizeHintForColumn(column);
}

void core::QAbstractItemViewWrap::setCurrentIndex(const QModelIndex& index) {
  m_wrapped.value<QAbstractItemView*>()->setCurrentIndex(index);
}

QScrollBar* core::QAbstractItemViewWrap::verticalScrollBar() const {
  return m_wrapped.value<QAbstractItemView*>()->verticalScrollBar();
}

bool core::QAbstractItemViewWrap::isVisible() const {
  return m_wrapped.value<QAbstractItemView*>()->isVisible();
}

void core::QAbstractItemViewWrap::hide() {
  return m_wrapped.value<QAbstractItemView*>()->hide();
}

void core::QAbstractItemViewWrap::setSelectionModel(QItemSelectionModel* selectionModel) {
  m_wrapped.value<QAbstractItemView*>()->setSelectionModel(selectionModel);
}

QItemSelectionModel* core::QAbstractItemViewWrap::selectionModel() const {
  return m_wrapped.value<QAbstractItemView*>()->selectionModel();
}
