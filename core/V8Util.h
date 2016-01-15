#pragma once

#include <v8.h>
#include <string>
#include <QString>
#include <QVariant>
#include <QCache>
#include <QMultiHash>

#include "macros.h"

namespace core {

typedef QList<QByteArray> ParameterTypes;
typedef std::pair<int, ParameterTypes> MethodInfo;

// static class
class V8Util {
  V8Util() = delete;
  ~V8Util() = delete;

 public:
  static QString toQString(v8::Local<v8::String> str) {
    return QString::fromUtf16(*v8::String::Value(str));
  }

  static std::string toStdString(v8::Local<v8::String> str) {
    v8::String::Utf8Value value(str);
    return *value;
  }

  static v8::Local<v8::String> toV8String(v8::Isolate* isolate, const QString& str) {
    return v8::String::NewFromTwoByte(isolate, str.utf16());
  }

  static v8::Local<v8::String> toV8String(v8::Isolate* isolate, const std::string& str) {
    return v8::String::NewFromUtf8(isolate, str.c_str());
  }

  static QVariant toVariant(v8::Isolate* isolate, v8::Local<v8::Value> value);

  static v8::Local<v8::Value> toV8Value(v8::Isolate* isolate, const QVariant& var);

  static v8::Local<v8::Value> toV8ObjectFrom(v8::Isolate* isolate, QObject* sourceObj);

  static void throwError(v8::Isolate* isolate, const std::string& msg);
  static void throwError(v8::Isolate* isolate, const char* msg);

  static void invokeMethod(const v8::FunctionCallbackInfo<v8::Value>& args);

 private:
  static QCache<const QMetaObject*, QMultiHash<QString, MethodInfo>> s_classMethodCache;

  static QVariant invokeMethodInternal(v8::Isolate* isolate,
                                       QObject* object,
                                       const QString& methodName,
                                       QVariantList args);
  static void cacheMethods(const QMetaObject* metaObj);
};

}  // namespace core
