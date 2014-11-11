#pragma once

#include "macros.h"
#include "ICursorDrawer.h"

class DefaultCursorDrawer : public ICursorDrawer {
  DISABLE_COPY(DefaultCursorDrawer)
 public:
  DefaultCursorDrawer() = default;
  ~DefaultCursorDrawer() = default;
  DEFAULT_MOVE(DefaultCursorDrawer)

  std::tuple<QRect, QColor> draw(const QRect& cursorRect) override;
};
