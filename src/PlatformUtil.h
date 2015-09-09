#pragma once

#include <yaml-cpp/yaml.h>
#include <QString>

#include "macros.h"

class Window;
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
#ifdef Q_OS_MAC
  static void parseMenuNode(const std::string& pkgName, const YAML::Node& menuNode);
#elif defined Q_OS_WIN
  static void parseMenuNode(const std::string& pkgName,
                            const YAML::Node& menuNode,
                            QList<Window*> windows);
#endif

 private:
  PlatformUtil() = delete;
  ~PlatformUtil() = delete;
};
