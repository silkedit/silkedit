#pragma once

#include <qmetatype.h>
#include <QPushButton>
#include <QLayout>

class MetaTypeInitializer {
 public:
  static void init();

  MetaTypeInitializer() = delete;
  ~MetaTypeInitializer() = delete;
};

Q_DECLARE_METATYPE(QPushButton*)
Q_DECLARE_METATYPE(QLayout*)
