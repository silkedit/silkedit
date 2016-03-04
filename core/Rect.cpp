#include "Rect.h"

namespace core {

Rect::Rect() {
  m_wrapped = QVariant::fromValue(QRect());
}

Rect::Rect(QRect rect) {
  m_wrapped = QVariant::fromValue(rect);
}

int Rect::width() const Q_DECL_NOTHROW {
  return m_wrapped.value<QRect>().width();
}

int Rect::height() const Q_DECL_NOTHROW {
  return m_wrapped.value<QRect>().height();
}

void Rect::setWidth(int w) Q_DECL_NOTHROW {
  auto rect = m_wrapped.value<QRect>();
  rect.setWidth(w);
  m_wrapped = QVariant::fromValue(rect);
}

void Rect::setHeight(int h) Q_DECL_NOTHROW {
  auto rect = m_wrapped.value<QRect>();
  rect.setHeight(h);
  m_wrapped = QVariant::fromValue(rect);
}

}  // namespace core
