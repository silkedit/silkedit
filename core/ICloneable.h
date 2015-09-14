#pragma once

#include "macros.h"

namespace core {

template <class T>
class ICloneable {
  DISABLE_COPY_AND_MOVE(ICloneable)

 public:
  ICloneable() = default;
  virtual ~ICloneable() = default;

  virtual T* clone() = 0;
};

}  // namespace core
