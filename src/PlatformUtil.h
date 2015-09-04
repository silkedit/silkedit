#pragma once

#include <yaml-cpp/yaml.h>
#include <QString>

#include "macros.h"

class PlatformUtil {
  DISABLE_COPY_AND_MOVE(PlatformUtil)

 public:
  static void showInFinder(const QString& filePath);
  static QString showInFinderText();
  static void enableMnemonicOnMac();

  /**
   * @brief Parse a menu node and add menus
   * @param menuNode
   */
  static void parseMenuNode(const YAML::Node& menuNode);

 private:
  PlatformUtil() = delete;
  ~PlatformUtil() = delete;
};
