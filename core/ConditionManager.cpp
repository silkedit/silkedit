#include "ConditionManager.h"
#include "OSCondition.h"

using core::ICondition;

std::unordered_map<QString, std::unique_ptr<ICondition>> core::ConditionManager::s_conditions;

void core::ConditionManager::init() {
  s_conditions.clear();
  // register default conditions
  add(OSCondition::name, std::move(std::unique_ptr<ICondition>(new OSCondition())));
}

void core::ConditionManager::add(const QString& key, std::unique_ptr<core::ICondition> condition) {
  s_conditions[key] = std::move(condition);
}

void core::ConditionManager::remove(const QString& key) {
  s_conditions.erase(key);
}

bool core::ConditionManager::isSatisfied(const QString& key,
                                         core::Operator op,
                                         const QString& value) {
  if (s_conditions.find(key) == s_conditions.end())
    return false;

  return s_conditions.at(key)->isSatisfied(op, value);
}

bool core::ConditionManager::isStatic(const QString& key) {
  if (s_conditions.find(key) == s_conditions.end())
    return false;

  return s_conditions.at(key)->isStatic();
}
