#pragma once

#include <yaml-cpp/yaml.h>
#include <QWidget>

#include "macros.h"

class Context;

class YamlUtils {
  DISABLE_COPY_AND_MOVE(YamlUtils)

 public:
  static Context* parseContext(const YAML::Node& contextNode);
  static void parseMenuNode(const std::string& pkgName,
                            QWidget* parent,
                            const YAML::Node& menuNode);

 private:
  YamlUtils() = delete;
  ~YamlUtils() = delete;
};
