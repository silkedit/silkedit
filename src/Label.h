#pragma once

#include <QLabel>

#include "core/macros.h"

class Label : public QLabel {
  Q_OBJECT
  DISABLE_COPY(Label)

 public:
  Q_INVOKABLE Label(QWidget* parent = 0, Qt::WindowFlags f = 0) ;
  Q_INVOKABLE Label(const QString& text, QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~Label();
  DEFAULT_MOVE(Label)
};
