#pragma once

#include <boost/optional.hpp>
#include <qmetatype.h>
#include <QString>
#include <QMetaProperty>
#include <QPushButton>

class MetaTypeInitializer {
 public:
  static void init();

  MetaTypeInitializer() = delete;
  ~MetaTypeInitializer() = delete;
};

Q_DECLARE_METATYPE(boost::optional<QString>)
Q_DECLARE_METATYPE(QMetaProperty)
Q_DECLARE_METATYPE(QPushButton*)
