#pragma once

#include <unordered_map>
#include <memory>
#include <QString>

#include "macros.h"
#include "ICondition.h"
#include "stlSpecialization.h"
#include "ConditionExpression.h"

namespace core {

class ConditionManager {
  DISABLE_COPY_AND_MOVE(ConditionManager)

 public:
  static void init();
  static void add(const QString& key, std::unique_ptr<ICondition> condition);
  static void remove(const QString& key);
  static bool isSatisfied(const QString& key, core::Operator op, const QString& value);
  static bool isStatic(const QString& key);

 private:
  static std::unordered_map<QString, std::unique_ptr<ICondition>> s_conditions;

  ConditionManager() = default;
  ~ConditionManager() = default;
};

}  // namespace core
