#include "ConditionExpression.h"
#include "PluginCondition.h"
#include "core/OSCondition.h"
#include "core/ICondition.h"

using core::ICondition;
using core::Operator;
using core::OSCondition;

std::unordered_map<QString, std::unique_ptr<ICondition>> ConditionExpression::s_conditions;

void ConditionExpression::init() {
  s_conditions.clear();
  // register default conditions
  add(OSCondition::name, std::move(std::unique_ptr<ICondition>(new OSCondition())));
}

void ConditionExpression::add(const QString& key, std::unique_ptr<ICondition> condition) {
  s_conditions[key] = std::move(condition);
}

void ConditionExpression::remove(const QString& key) {
  s_conditions.erase(key);
}

ConditionExpression::ConditionExpression(const QString& key, Operator op, const QString& value)
    : m_key(key), m_op(op), m_value(value) {}

bool ConditionExpression::isSatisfied() {
  if (s_conditions.find(m_key) == s_conditions.end())
    return false;

  return s_conditions.at(m_key)->isSatisfied(m_op, m_value);
}

QString ConditionExpression::toString() {
  return QString("%1 %2 %3").arg(m_key).arg(ICondition::operatorString(m_op)).arg(m_value);
}

bool ConditionExpression::isStatic() {
  if (s_conditions.find(m_key) == s_conditions.end())
    return false;

  return s_conditions.at(m_key)->isStatic();
}

bool ConditionExpression::operator==(const ConditionExpression& other) const {
  return this->m_key == other.m_key && this->m_op == other.m_op && this->m_value == other.m_value;
}
