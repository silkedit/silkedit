#include "ConditionExpression.h"
#include "OSCondition.h"
#include "Condition.h"

using core::Condition;

core::ConditionExpression::ConditionExpression(const QString& key,
                                               Condition::Operator op,
                                               const QVariant &value)
    : m_key(key), m_op(op), m_value(value) {}

bool core::ConditionExpression::isSatisfied() const {
  return Condition::isSatisfied(m_key, m_op, m_value);
}

QString core::ConditionExpression::toString() const {
  if (m_op == Condition::Operator::EQUALS && m_value == "true") {
    return QString("%1").arg(m_key);
  } else {
    return QString("%1 %2 %3").arg(m_key).arg(Condition::operatorString(m_op)).arg(m_value.toString());
  }
}

bool core::ConditionExpression::isStatic() const {
  return Condition::isStatic(m_key);
}

bool core::ConditionExpression::operator==(const core::ConditionExpression& other) const {
  return this->m_key == other.m_key && this->m_op == other.m_op && this->m_value == other.m_value;
}
