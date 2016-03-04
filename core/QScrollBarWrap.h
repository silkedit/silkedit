#pragma once

#include <QScrollBar>

#include "Wrapper.h"

namespace core {

class QScrollBarWrap : public Wrapper {
  Q_OBJECT
  Q_CLASSINFO(WRAPPED, "QScrollBar*")

 public:
  QScrollBarWrap(QScrollBar* sb);
  ~QScrollBarWrap() = default;

 public slots:
  QSize sizeHint() const;

 private:
};

}  // namespace core

Q_DECLARE_METATYPE(core::QScrollBarWrap*)
Q_DECLARE_METATYPE(QScrollBar*)
