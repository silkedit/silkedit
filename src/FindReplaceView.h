#pragma once

#include <QWidget>

#include "macros.h"

class FindReplaceView : public QWidget {
  DISABLE_COPY(FindReplaceView)

 public:
  explicit FindReplaceView(QWidget* parent);
  ~FindReplaceView() = default;
  DEFAULT_MOVE(FindReplaceView)

 private:
};
