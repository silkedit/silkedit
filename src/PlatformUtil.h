#pragma once

#include <yaml-cpp/yaml.h>
#include <QString>

#include "core/macros.h"

class Window;
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
