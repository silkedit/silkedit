#include "ConditionExpression.h"
#include "OSCondition.h"
#include "ICondition.h"
#include "ConditionManager.h"

using core::ICondition;

core::ConditionExpression::ConditionExpression(const QString& key,
                                               Operator op,
                                               const QString& value)
    : m_key(key), m_op(op), m_value(value) {}

bool core::ConditionExpression::isSatisfied() const {
  return ConditionManager::isSatisfied(m_key, m_op, m_value);
}

QString core::ConditionExpression::toString() const {
  if (m_op == Operator::EQUALS && m_value == "true") {
    return QString("%1").arg(m_key);
  } else {
    return QString("%1 %2 %3").arg(m_key).arg(ICondition::operatorString(m_op)).arg(m_value);
  }
}

bool core::ConditionExpression::isStatic() const {
  return ConditionManager::isStatic(m_key);
}

bool core::ConditionExpression::operator==(const core::ConditionExpression& other) const {
  return this->m_key == other.m_key && this->m_op == other.m_op && this->m_value == other.m_value;
}
