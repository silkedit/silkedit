#pragma once

#include <QKeyEvent>

#include "Wrapper.h"

namespace core {

class KeyEvent : public Wrapper {
  Q_OBJECT
  Q_CLASSINFO(WRAPPED, "QKeyEvent*")
  Q_CLASSINFO(INHERITS, "QEvent")

 public:
  Q_INVOKABLE KeyEvent(QEvent::Type type,
                       int key,
                       int modifiers,
                       const QString& text = QString(),
                       bool autorep = false,
                       int count = 1);
  explicit KeyEvent(QKeyEvent* event);

  ~KeyEvent() = default;

 public slots:
  int type() const { return static_cast<int>(m_wrapped.value<QKeyEvent*>()->type()); }
  int key() const { return m_wrapped.value<QKeyEvent*>()->key(); }

 private:
};

}  // namespace core

Q_DECLARE_METATYPE(core::KeyEvent*)
Q_DECLARE_METATYPE(QKeyEvent*)
