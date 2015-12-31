#pragma once

#include <v8.h>
#include <QObject>

constexpr int MAX_ARGS_COUNT = 10;

class JSHandler {
 public:
  static void init(v8::Local<v8::Object> jsHandler);
  static QVariant callFunc(const QString& funcName, QVariantList args);
  static void inheritsQtEventEmitter(v8::Local<v8::Value> proto);
  static void emitSignal(QObject *obj, const QString& signal, QVariantList args);

 private:
  static v8::Persistent<v8::Object> s_jsHandler;

  JSHandler() = delete;
  ~JSHandler() = delete;
};

