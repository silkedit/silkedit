#include "QScrollBarWrap.h"

namespace core {

QScrollBarWrap::QScrollBarWrap(QScrollBar* sb) {
  m_wrapped = QVariant::fromValue(sb);
}

QSize QScrollBarWrap::sizeHint() const {
  return m_wrapped.value<QScrollBar*>()->sizeHint();
}

}  // namespace core
