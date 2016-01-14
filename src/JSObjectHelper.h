#pragma once

#include <v8.h>
#include <QVariant>
#include <QThreadStorage>

#include "core/Singleton.h"

typedef QList<QByteArray> ParameterTypes;
typedef std::pair<int, ParameterTypes> MethodInfo;

class JSObjectHelper : public QObject, public core::Singleton<JSObjectHelper> {
  Q_OBJECT

 public:
  static QVariant toVariant(v8::Isolate* isolate, v8::Local<v8::Value> value);

  static v8::Local<v8::Value> toV8Value(v8::Isolate* isolate, const QVariant& var);
  static v8::Local<v8::Value> toV8ObjectFrom(v8::Isolate* isolate, QObject* sourceObj);
  static void invokeMethod(const v8::FunctionCallbackInfo<v8::Value>& args);
  static bool matchTypes(QList<QByteArray> types, QVariantList args);
  static void connect(const v8::FunctionCallbackInfo<v8::Value>& info);

  ~JSObjectHelper() = default;

 private:
  static QCache<const QMetaObject*, QMultiHash<QString, MethodInfo>> s_classMethodCache;

  static v8::Local<v8::Value> toV8ValueInternal(const QVariant& var,
                                                v8::Isolate* isolate = v8::Isolate::GetCurrent());

  //  Throws exception
  static QVariant invokeMethodInternal(v8::Isolate* isolate,
                                       QObject* object,
                                       const QString& methodName,
                                       QVariantList args);
  static void cacheMethods(const QMetaObject* metaObj);

  friend class core::Singleton<JSObjectHelper>;
  JSObjectHelper() = default;
};
