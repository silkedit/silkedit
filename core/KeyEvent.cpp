#include "KeyEvent.h"

namespace core {
KeyEvent::KeyEvent(QEvent::Type type,
                   int key,
                   int modifiers,
                   const QString& text,
                   bool autorep,
                   int count) {
  m_wrapped =
      QVariant::fromValue(new QKeyEvent(type, key, static_cast<Qt::KeyboardModifiers>(modifiers),
                                        text, autorep, static_cast<ushort>(count)));
}

KeyEvent::KeyEvent(QKeyEvent* event) {
  m_wrapped = QVariant::fromValue(event);
}

}  // namespace core
