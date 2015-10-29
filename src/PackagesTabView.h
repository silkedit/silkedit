#pragma once

#include <QTabWidget>

#include "core/macros.h"

class PackagesTabView : public QTabWidget {
  DISABLE_COPY(PackagesTabView)

 public:
  PackagesTabView();
  ~PackagesTabView() = default;
  DEFAULT_MOVE(PackagesTabView)

 private:
};
