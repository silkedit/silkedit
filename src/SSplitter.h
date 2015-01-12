#pragma once

#include <QSplitter>

#include "macros.h"

class SSplitter : public QSplitter {
  Q_OBJECT
  DISABLE_COPY(SSplitter)

 public:
  SSplitter(Qt::Orientation orientation, QWidget* parent = nullptr);
  ~SSplitter() = default;
  DEFAULT_MOVE(SSplitter)
};

class SHSplitter : public SSplitter {
  Q_OBJECT

  public:
   SHSplitter(QWidget* parent): SSplitter(Qt::Orientation::Horizontal, parent){}
   ~SHSplitter() = default;
};
