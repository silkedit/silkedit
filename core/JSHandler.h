#pragma once

#include <v8.h>
#include <QObject>
#include <QVariant>

namespace core {

class JSHandler {
 public:
  static void init(v8::Local<v8::Object> jsHandler);
  static QVariant callFunc(v8::Isolate *isolate, const QString& funcName, QVariantList args);
  static void inheritsQtEventEmitter(v8::Isolate *isolate, v8::Local<v8::Value> proto);
  static void emitSignal(v8::Isolate* isolate, QObject *obj, const QString& signal, QVariantList args);

  template <typename T>
  static T callFunc(v8::Isolate* isolate, const QString &funcName, QVariantList args, T defaultValue) {
    QVariant result = callFunc(isolate, funcName, args);
    return result.canConvert<T>() ? result.value<T>() : defaultValue;
  }

private:
  static v8::Persistent<v8::Object> s_jsHandler;
  static bool s_isInitialized;

  JSHandler() = delete;
  ~JSHandler() = delete;
};

}  // namespace core

