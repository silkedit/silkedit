#pragma once

#include <QObject>

#include "core/macros.h"

namespace core {

class JSNull {
 public:
  JSNull() = default;
  ~JSNull() = default;

 private:
};

}  // namespace core

Q_DECLARE_METATYPE(core::JSNull)
