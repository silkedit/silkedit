#pragma once

#include <QToolBar>

#include "core/macros.h"

class ToolBar : public QToolBar {
  DISABLE_COPY(ToolBar)

 public:
  ToolBar(const QString& objectName, const QString title, QWidget* parent);
  ~ToolBar() = default;
  DEFAULT_MOVE(ToolBar)

 private:
};
