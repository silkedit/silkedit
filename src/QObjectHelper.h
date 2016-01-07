#pragma once

#include <node.h>
#include <memory>
#include <QObject>
#include <QMetaMethod>

#include "core/Singleton.h"
#include "core/macros.h"

// lives in main thread
class NODE_EXTERN QObjectHelper : public QObject, public core::Singleton<QObjectHelper> {
  Q_OBJECT
  DISABLE_COPY(QObjectHelper)

 public:
  ~QObjectHelper() = default;
  DEFAULT_MOVE(QObjectHelper)

 public slots:
  QObject* newInstanceFromJS(const QMetaObject& metaObj, QVariantList args);

  QVariant read(QObject* obj, const QMetaProperty& prop);
  void write(QObject* obj, const QMetaProperty& prop, const QVariant& value);

 private:
  friend class core::Singleton<QObjectHelper>;
  QObjectHelper() = default;
};
