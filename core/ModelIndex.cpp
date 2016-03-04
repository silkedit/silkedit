#include "ModelIndex.h"

namespace core {

ModelIndex::ModelIndex() {
  m_wrapped = QVariant::fromValue(QModelIndex());
}

ModelIndex::ModelIndex(QModelIndex index) {
  m_wrapped = QVariant::fromValue(index);
}

}  // namespace core
