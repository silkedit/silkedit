#pragma once

#include <yaml-cpp/yaml.h>
#include <QWidget>

#include "core/macros.h"
#include "core/Config.h"
#include "core/ConfigDefinition.h"

class Context;
class Window;

class YamlUtils {
  DISABLE_COPY_AND_MOVE(YamlUtils)

 public:
  static Context* parseContext(const YAML::Node& contextNode);
  static void parseMenuNode(const QString& pkgName, QWidget* parent, const YAML::Node& menuNode);
  static void parseToolbarNode(const QString& pkgName,
                               const std::string& ymlPath,
                               QWidget* window,
                               const YAML::Node& toolbarNode);
  static QList<core::ConfigDefinition> parseConfig(const QString& pkgName,
                                                   const std::string& ymlPath);

 private:
  YamlUtils() = delete;
  ~YamlUtils() = delete;
};
