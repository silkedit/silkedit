#pragma once

#include <QMetaType>
#include <v8.h>

#include "macros.h"

namespace core {

struct FunctionInfo {
  v8::Isolate* isolate;
  v8::Local<v8::Function> fn;
};

}  // namespace core

Q_DECLARE_METATYPE(core::FunctionInfo)
