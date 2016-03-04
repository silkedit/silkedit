#include "QSizeWrap.h"

namespace core {

QSizeWrap::QSizeWrap() {
  m_wrapped = QVariant::fromValue(QSize());
}

QSizeWrap::QSizeWrap(int width, int height) {
  m_wrapped = QVariant::fromValue(QSize(width, height));
}

QSizeWrap::QSizeWrap(QSize size) {
  m_wrapped = QVariant::fromValue(size);
}

int QSizeWrap::width() const Q_DECL_NOTHROW {
  return m_wrapped.value<QSize>().width();
}

int QSizeWrap::height() const Q_DECL_NOTHROW {
  return m_wrapped.value<QSize>().height();
}

}  // namespace core
