#pragma once

#include <memory>
#include <unordered_map>
#include <QString>
#include <QHash>

#include "Condition.h"
#include "stlSpecialization.h"

namespace core {

struct ConditionExpression {
  QString m_key;
  core::Condition::Operator m_op;
  QString m_value;

  ConditionExpression(const QString& key, core::Condition::Operator op, const QString& value);
  bool isSatisfied() const;
  QString toString() const;

  /**
   * @brief check if condition is static (e.g., os == mac)
   * @return
   */
  bool isStatic() const;
  bool operator==(const ConditionExpression& other) const;

  bool operator!=(const ConditionExpression& other) const { return !(*this == other); }
};

inline uint qHash(const ConditionExpression& cond, uint seed) {
  return qHash(cond.toString(), seed);
}

}  // namespace core
