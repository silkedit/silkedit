#pragma once

#include <memory>
#include <QObject>
#include <QMetaMethod>

#include "macros.h"

namespace core {

// static class
class QObjectUtil {
  QObjectUtil() = delete;
  ~QObjectUtil() = delete;

 public:
  static QObject* newInstanceFromJS(const QMetaObject& metaObj, QVariantList args);
  static void* newInstanceOfGadgetFromJS(const QMetaObject& metaObj, QVariantList args);
};

}  // namespace core

