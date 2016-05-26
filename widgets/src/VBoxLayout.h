#pragma once

#include <QVBoxLayout>

#include "core/macros.h"

class VBoxLayout : public QVBoxLayout {
  Q_OBJECT
  DISABLE_COPY(VBoxLayout)

 public:
  Q_INVOKABLE VBoxLayout();
  Q_INVOKABLE explicit VBoxLayout(QWidget* parent);
  ~VBoxLayout();
  DEFAULT_MOVE(VBoxLayout)

 public slots:
  void addWidget(QWidget* widget, int stretch = 0, Qt::Alignment alignment = 0);
};
