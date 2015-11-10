#pragma once

#include <memory>
#include <unordered_map>
#include <QString>

#include "core/ICondition.h"
#include "core/stlSpecialization.h"

class ConditionExpression {
 public:
  static void init();
  static void add(const QString& key, std::unique_ptr<core::ICondition> condition);
  static void remove(const QString& key);

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

 private:
  static std::unordered_map<QString, std::unique_ptr<core::ICondition>> s_conditions;
};
