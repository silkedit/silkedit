#pragma once

#include "macros.h"

class PlatformUtil {
  DISABLE_COPY_AND_MOVE(PlatformUtil)

 public:
  static void showInFinder(const QString& filePath);
  static QString showInFinderText();

 private:
  PlatformUtil() = delete;
  ~PlatformUtil() = delete;
};
