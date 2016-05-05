#include "ConditionExpression.h"
#include "ConditionManager.h"

namespace core {

ConditionExpression::ConditionExpression(const QString& key,
                                               const QString &op,
                                               const QVariant &operand)
    : m_key(key), m_operator(op), m_operand(operand) {}

bool ConditionExpression::isSatisfied() const {
  return ConditionManager::singleton().isSatisfied(m_key, m_operator, m_operand);
}

QString ConditionExpression::toString() const {
  if (m_operator == Condition::equalsOperator && m_operand == "true") {
    return QString("%1").arg(m_key);
  } else {
    return QString("%1 %2 %3").arg(m_key).arg(m_operator).arg(m_operand.toString());
  }
}

bool ConditionExpression::isStatic() const {
  return ConditionManager::singleton().isStatic(m_key);
}

bool ConditionExpression::operator==(const ConditionExpression& other) const {
  return this->m_key == other.m_key && this->m_operator == other.m_operator && this->m_operand == other.m_operand;
}

}  // namespace core

