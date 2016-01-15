#pragma once

#include <v8.h>
#include <QObject>

#include "core/Singleton.h"

class JSObjectHelper : public QObject, public core::Singleton<JSObjectHelper> {
  Q_OBJECT

 public:
  static void connect(const v8::FunctionCallbackInfo<v8::Value>& info);

  ~JSObjectHelper() = default;

 private:
  friend class core::Singleton<JSObjectHelper>;
  JSObjectHelper() = default;
};
