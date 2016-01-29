#pragma once

#include <memory>
#include <QObject>
#include <QMetaMethod>
#include <QCache>

#include "macros.h"
#include "Util.h"

namespace core {

typedef std::pair<int, ParameterTypes> MethodInfo;

// static class
class QObjectUtil {
  QObjectUtil() = delete;
  ~QObjectUtil() = delete;

 public:
  static QVariant invokeQObjectMethodInternal(QObject* object,
                                              const QString& methodName,
                                              QVariantList args);
  static QObject* newInstanceFromJS(const QMetaObject& metaObj, QVariantList args);
  static void* newInstanceOfGadgetFromJS(const QMetaObject& metaObj, QVariantList args);

 private:
  static QCache<const QMetaObject*, QMultiHash<QString, MethodInfo>> s_classMethodCache;

  static void cacheMethods(const QMetaObject* metaObj);
};

}  // namespace core
