#pragma once

#include <functional>

#include "macros.h"

class Util {
  DISABLE_COPY_AND_MOVE(Util)

 public:
  static int binarySearch(int last, std::function<bool(int)> fn);
  static void ensureDir(const QString& path);

  /**
   * @brief Copy source to dist
   *
   * Creats parent directories of dist if they don't exist.
   *
   * @param source
   * @param dist
   * @return
   */
  static bool copy(const QString& source, const QString& dist);

 private:
  Util() = delete;
  ~Util() = delete;
};
