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

/**
 * @brief Parse a menu node and add menus
 * @param menuNode
 */
#ifdef Q_OS_MAC
  static void parseMenusNode(const std::string& pkgName, const YAML::Node& menusNode);
#elif defined Q_OS_WIN
  static void parseMenusNode(const std::string& pkgName,
                             const YAML::Node& menusNode,
                             QList<Window*> windows);
#endif

 private:
  PlatformUtil() = delete;
  ~PlatformUtil() = delete;
};
