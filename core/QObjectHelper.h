#pragma once

#include <memory>
#include <QObject>
#include <QMetaMethod>

#include "Singleton.h"
#include "macros.h"

namespace core {

class QObjectHelper : public QObject, public core::Singleton<QObjectHelper> {
  Q_OBJECT
  DISABLE_COPY(QObjectHelper)

 public:
  ~QObjectHelper() = default;
  DEFAULT_MOVE(QObjectHelper)

  QObject* newInstanceFromJS(const QMetaObject& metaObj, QVariantList args);
  void* newInstanceOfGadgetFromJS(const QMetaObject& metaObj, QVariantList args);
  QVariant read(QObject* obj, const QMetaProperty& prop);
  void write(QObject* obj, const QMetaProperty& prop, const QVariant& value);

 private:
  friend class core::Singleton<QObjectHelper>;
  QObjectHelper() = default;

  void* newInstanceOfGadget(const QMetaObject& metaObj, QGenericArgument val0=  QGenericArgument(0),
                            QGenericArgument val1 = QGenericArgument(),
                            QGenericArgument val2 = QGenericArgument(),
                            QGenericArgument val3 = QGenericArgument(),
                            QGenericArgument val4 = QGenericArgument(),
                            QGenericArgument val5 = QGenericArgument(),
                            QGenericArgument val6 = QGenericArgument(),
                            QGenericArgument val7 = QGenericArgument(),
                            QGenericArgument val8 = QGenericArgument(),
                            QGenericArgument val9 = QGenericArgument()) const;
};

}  // namespace core

