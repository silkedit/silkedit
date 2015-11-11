#include <algorithm>
#include <assert.h>

#include "AndConditionExpression.h"

core::AndConditionExpression::AndConditionExpression(QSet<core::ConditionExpression> condSet)
    : m_condSet(condSet) {
  assert(!m_condSet.isEmpty());
}

bool core::AndConditionExpression::isSatisfied() {
  return std::all_of(m_condSet.constBegin(), m_condSet.constEnd(),
                     [=](const ConditionExpression& cond) { return cond.isSatisfied(); });
}

QString core::AndConditionExpression::toString() {
  QStringList strs;
  for (const auto& cond : m_condSet) {
    strs.append(cond.toString());
  }

  return strs.join(" && ").trimmed();
}

bool core::AndConditionExpression::isStatic() const {
  return std::all_of(m_condSet.constBegin(), m_condSet.constEnd(),
                     [=](const ConditionExpression& cond) { return cond.isStatic(); });
}

int core::AndConditionExpression::size() {
  return m_condSet.size();
}

bool core::AndConditionExpression::operator==(const core::AndConditionExpression& other) const {
  return m_condSet == other.m_condSet;
}
