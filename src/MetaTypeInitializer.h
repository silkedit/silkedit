#pragma once

#include <boost/optional.hpp>
#include <qmetatype.h>
#include <QString>
#include <QMetaProperty>
#include <QPushButton>
#include <QLayout>

class MetaTypeInitializer {
 public:
  static void init();

  MetaTypeInitializer() = delete;
  ~MetaTypeInitializer() = delete;
};

Q_DECLARE_METATYPE(boost::optional<QString>)
Q_DECLARE_METATYPE(QMetaProperty)
Q_DECLARE_METATYPE(QPushButton*)
Q_DECLARE_METATYPE(QLayout*)
