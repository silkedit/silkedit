#pragma once

#include "macros.h"

class QKeyEvent;

class IKeyEventFilter {
  DISABLE_COPY_AND_MOVE(IKeyEventFilter)

 public:
  IKeyEventFilter() = default;
  virtual ~IKeyEventFilter() = default;

  virtual bool keyEventFilter(QKeyEvent* event) = 0;
};
