#pragma once

#include <functional>

#include "macros.h"

class Util {
  DISABLE_COPY_AND_MOVE(Util)

 public:
  static int binarySearch(int last, std::function<bool(int)> fn);

 private:
  Util() = delete;
  ~Util() = delete;
};
