#pragma once

#include <QString>
#include <QVariant>

#include "Condition.h"

namespace core {

struct ConditionExpression {
  QString m_key;
  Condition::Operator m_op;
  QVariant m_value;

  ConditionExpression(const QString& key, Condition::Operator op, const QVariant& value);
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
