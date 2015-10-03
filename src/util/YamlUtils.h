#pragma once

#include <yaml-cpp/yaml.h>
#include <QWidget>

#include "core/macros.h"

class Context;
class Window;

class YamlUtils {
  DISABLE_COPY_AND_MOVE(YamlUtils)

 public:
  static Context* parseContext(const YAML::Node& contextNode);
  static void parseMenusNode(const std::string& pkgName,
                             QWidget* parent,
                             const YAML::Node& menusNode);
  static void parseToolbarsNode(const std::string& pkgName,
                                const std::string& ymlPath,
                                QWidget* window,
                                const YAML::Node& toolbarsNode);

 private:
  YamlUtils() = delete;
  ~YamlUtils() = delete;
};
