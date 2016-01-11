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

  static void init(v8::Local<v8::Object> exports, v8::Local<v8::Value> unused, v8::Local<v8::Context> context, void *priv);
  static void lateInit(const v8::FunctionCallbackInfo<v8::Value>& args);

 private:
  static void setSingletonObj(v8::Local<v8::Object>& exports, QObject* sourceObj, const char* name);
};

}  // namespace bridge
