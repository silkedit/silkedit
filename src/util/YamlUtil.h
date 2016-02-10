#pragma once

#include <boost/optional.hpp>
#include <yaml-cpp/yaml.h>
#include <QWidget>

#include "core/macros.h"
#include "core/Config.h"
#include "core/ConfigDefinition.h"
#include "core/AndConditionExpression.h"

namespace core {
struct ConditionExpression;
class AndConditionExpression;
}
class Window;

class YamlUtil {
  DISABLE_COPY_AND_MOVE(YamlUtil)

 public:
  static boost::optional<core::AndConditionExpression> parseCondition(
      const YAML::Node& conditionNode);
  static boost::optional<core::ConditionExpression> parseValueCondition(const QString& str);
  static void parseMenuNode(const QString& pkgName, QWidget* parent, const YAML::Node& menuNode);
  static void parseToolbarNode(const QString& pkgName,
                               const QString& ymlPath,
                               QWidget* window,
                               const YAML::Node& toolbarNode);
  static QList<core::ConfigDefinition> parseConfig(const QString& pkgName, const QString& ymlPath);

 private:
  YamlUtil() = delete;
  ~YamlUtil() = delete;
};
