#pragma once

#include <QTabWidget>
#include <QObject>

#include "core/macros.h"

class PackagesTabView : public QTabWidget {
  Q_OBJECT
  DISABLE_COPY(PackagesTabView)

 public:
  PackagesTabView();
  ~PackagesTabView() = default;
  DEFAULT_MOVE(PackagesTabView)

 private:
};
