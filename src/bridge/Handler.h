#pragma once

#include <v8.h>
#include <QObject>

#include "core/macros.h"

namespace bridge {

class Handler {
  DISABLE_COPY(Handler)

 public:
  Handler() = delete;
  ~Handler() = delete;
  DEFAULT_MOVE(Handler)

  static void init(v8::Local<v8::Object> exports,
                   v8::Local<v8::Value> unused,
                   v8::Local<v8::Context> context,
                   void* priv);
  static void lateInit(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void info(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void warn(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void error(const v8::FunctionCallbackInfo<v8::Value>& args);

 private:
  static void setSingletonObj(v8::Local<v8::Object>& exports, QObject* sourceObj, const char* name);
  static void registerStaticMethods(const QMetaObject& metaObj, v8::Local<v8::Function> ctor);

  template <typename T>
  static void registerClass(v8::Local<v8::Object> exports);

  template <typename T>
  static void registerQtEnum(v8::Local<v8::Context> context, v8::Local<v8::Object> exports, v8::Isolate* isolate, const char* name);
};

}  // namespace bridge
