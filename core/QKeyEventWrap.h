#pragma once

#include <vendor/node/src/node_object_wrap.h>
#include <QKeyEvent>

#include "macros.h"

namespace core {

class QKeyEventWrap : public node::ObjectWrap {
  Q_GADGET

 public:
  static v8::Persistent<v8::Function> constructor;

  static void Init(v8::Local<v8::Object> exports);

  QKeyEvent* keyEvent() { return m_keyEvent; }

  QKeyEventWrap(QKeyEvent* event);
  ~QKeyEventWrap() = default;

 private:

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void type(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void key(const v8::FunctionCallbackInfo<v8::Value>& args);

  QKeyEvent* m_keyEvent;
};

}  // namespace core

