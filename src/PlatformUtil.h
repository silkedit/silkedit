#pragma once

#include <QString>

#include "macros.h"

class PlatformUtil {
  DISABLE_COPY_AND_MOVE(PlatformUtil)

 public:
  static void showInFinder(const QString& filePath);
  static QString showInFinderText();
  static void enableMnemonicOnMac();

 private:
  PlatformUtil() = delete;
  ~PlatformUtil() = delete;
};
