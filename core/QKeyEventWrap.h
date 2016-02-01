#pragma once

#include <QKeyEvent>

#include "macros.h"
#include "Wrapper.h"

namespace core {

class QKeyEventWrap : public Wrapper {
  Q_OBJECT
  Q_CLASSINFO(WRAPPED, "QKeyEvent*")

 public:
  QKeyEventWrap(QKeyEvent* event) { m_wrapped = QVariant::fromValue(event); }

  ~QKeyEventWrap() = default;

 public slots:
  int type() const { return static_cast<int>(m_wrapped.value<QKeyEvent*>()->type()); }
  int key() const { return m_wrapped.value<QKeyEvent*>()->key(); }

 private:
};

}  // namespace core

Q_DECLARE_METATYPE(core::QKeyEventWrap*)
Q_DECLARE_METATYPE(QKeyEvent*)
