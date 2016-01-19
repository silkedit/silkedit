#pragma once

#include <vendor/node/src/node_object_wrap.h>
#include <QFont>

#include "macros.h"

namespace core {

class Font : public QFont, public node::ObjectWrap {
  Q_GADGET
  DISABLE_COPY(Font)

 public:
  static void Init(v8::Local<v8::Object> exports);

  Q_INVOKABLE Font(const QString& family, int pointSize = -1, int weight = -1, bool italic = false)
      : QFont(family, pointSize, weight, italic) {}
  ~Font() = default;
  DEFAULT_MOVE(Font)

 private:
  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
};

}  // namespace core

