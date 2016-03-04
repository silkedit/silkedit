#pragma once

#include <qmetatype.h>
#include <QPushButton>
#include <QLayout>
#include <QAbstractItemModel>

class MetaTypeInitializer {
 public:
  static void init();

  MetaTypeInitializer() = delete;
  ~MetaTypeInitializer() = delete;
};

Q_DECLARE_METATYPE(QPushButton*)
Q_DECLARE_METATYPE(QLayout*)
Q_DECLARE_METATYPE(QAbstractItemModel*)
Q_DECLARE_METATYPE(QEvent*)
