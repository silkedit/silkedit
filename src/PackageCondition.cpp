#include "PackageCondition.h"
#include "Helper.h"

using core::Operator;

PackageCondition::PackageCondition(const QString& key) : m_key(key) {}

bool PackageCondition::isSatisfied(Operator op, const QString& operand) {
  return Helper::singleton().askExternalCondition(m_key, op, operand);
}

QString PackageCondition::key() {
  throw std::runtime_error("this method should not be called");
}
