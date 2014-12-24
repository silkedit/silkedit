#pragma once

#include <QMenuBar>

#include "macros.h"

class MenuBar : public QMenuBar {
  DISABLE_COPY(MenuBar)

 public:
  MenuBar(QWidget* parent = nullptr);
  ~MenuBar() = default;
  DEFAULT_MOVE(MenuBar)

 private:
};
