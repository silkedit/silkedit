#pragma once

#include <QRect>
#include <QMetaType>

#include "Wrapper.h"

namespace core {

// Wrapper of QRect
class Rect : public Wrapper {
  Q_OBJECT
  Q_CLASSINFO(WRAPPED, "QRect")

 public:
  Q_INVOKABLE Rect();
  Rect(QRect rect);
  ~Rect() = default;

 public slots:
  int width() const Q_DECL_NOTHROW;
  int height() const Q_DECL_NOTHROW;
  void setWidth(int w) Q_DECL_NOTHROW;
  void setHeight(int h) Q_DECL_NOTHROW;

 private:
};

}  // namespace core

Q_DECLARE_METATYPE(core::Rect*)
Q_DECLARE_METATYPE(QRect)
