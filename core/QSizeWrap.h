#pragma once

#include <QSize>

#include "Wrapper.h"

namespace core {

class QSizeWrap : public Wrapper {
  Q_OBJECT
  Q_CLASSINFO(WRAPPED, "QSize")

 public:
  Q_INVOKABLE QSizeWrap();
  Q_INVOKABLE QSizeWrap(int width, int height);
  QSizeWrap(QSize size);
  ~QSizeWrap() = default;

 public slots:
  int width() const Q_DECL_NOTHROW;
  int height() const Q_DECL_NOTHROW;

 private:
};

}  // namespace core

Q_DECLARE_METATYPE(core::QSizeWrap*)
Q_DECLARE_METATYPE(QSize)
