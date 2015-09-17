#pragma once

#include "macros.h"

class QKeyEvent;

namespace core {

class IKeyEventFilter {
  DISABLE_COPY_AND_MOVE(IKeyEventFilter)

 public:
  IKeyEventFilter() = default;
  virtual ~IKeyEventFilter() = default;

  virtual bool keyEventFilter(QKeyEvent* event) = 0;
};

}  // namespace core
