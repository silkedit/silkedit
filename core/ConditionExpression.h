#pragma once

#include <QString>
#include <QVariant>

#include "Condition.h"

namespace core {

struct ConditionExpression {
  QString m_key;
  QString m_operator;
  QVariant m_operand;

  ConditionExpression(const QString& key, const QString& op, const QVariant& operand);
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
