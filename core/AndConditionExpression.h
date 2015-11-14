#pragma once

#include <QSet>

#include "macros.h"
#include "ConditionExpression.h"

namespace core {

class AndConditionExpression {
 public:
  AndConditionExpression(QSet<ConditionExpression> condSet);
  ~AndConditionExpression() = default;
  DEFAULT_COPY_AND_MOVE(AndConditionExpression)

  bool isSatisfied();
  QString toString();
  /**
   * @brief check if condition is static (e.g., os == mac)
   * @return
   */
  bool isStatic() const;
  int size();
  bool operator==(const AndConditionExpression& other) const;

  bool operator!=(const AndConditionExpression& other) const { return !(*this == other); }

 private:
  QSet<ConditionExpression> m_condSet;
};

}  // namespace core