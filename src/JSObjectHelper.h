#pragma once

#include <v8.h>
#include <QVariant>
#include <QThreadStorage>

#include "core/Singleton.h"

class JSObjectHelper : public QObject, public core::Singleton<JSObjectHelper> {
  Q_OBJECT

 public:
  static QVariant toVariant(v8::Local<v8::Value> value,
                            v8::Isolate* isolate = v8::Isolate::GetCurrent());

  static v8::Local<v8::Value> toV8Value(const QVariant& var, v8::Isolate* isolate);
  static void invokeMethod(const v8::FunctionCallbackInfo<v8::Value>& args);
  static bool matchTypes(QList<QByteArray> types, QVariantList args);
  static void connect(const v8::FunctionCallbackInfo<v8::Value> &info);

  ~JSObjectHelper() = default;

private:
  // todo: use QCache
  static QThreadStorage<QHash<QString, QHash<QString, int>>> m_classMethodHash;

  static v8::Local<v8::Value> toV8ValueInternal(const QVariant& var,
                                        v8::Isolate* isolate = v8::Isolate::GetCurrent());
  static v8::Local<v8::Value> toV8ObjectFrom(QObject *sourceObj, v8::Isolate *isolate);
  static QVariant invokeMethodInternal(QObject *object, const QString &methodName, QVariantList args);
  static void cacheMethods(const QString &className, const QMetaObject *metaObj);

  friend class core::Singleton<JSObjectHelper>;
  JSObjectHelper() = default;
};
