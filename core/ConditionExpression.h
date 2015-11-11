#pragma once

#include <memory>
#include <unordered_map>
#include <QString>

#include "ICondition.h"
#include "stlSpecialization.h"

namespace core {

struct ConditionExpression {
  QString m_key;
  core::Operator m_op;
  QString m_value;

  ConditionExpression(const QString& key, core::Operator op, const QString& value);
  bool isSatisfied();
  QString toString();

  /**
   * @brief check if condition is static (e.g., os == mac)
   * @return
   */
  bool isStatic();
  bool operator==(const ConditionExpression& other) const;

  bool operator!=(const ConditionExpression& other) const { return !(*this == other); }
};

}  // namespace core
