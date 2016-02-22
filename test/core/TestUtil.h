#pragma once

#include <QString>

#include "core/macros.h"

class TestUtil {
  DISABLE_COPY_AND_MOVE(TestUtil)

 public:
  static void compareLineByLine(const QString& str1, const QString& str2);

 private:
  TestUtil() = delete;
  ~TestUtil() = delete;
};
