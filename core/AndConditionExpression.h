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
   * @brief check if conditions has any static condition (e.g., os == mac)
   * @return
   */
  bool hasStatic() const;
  int size();
  bool operator==(const AndConditionExpression& other) const;

  bool operator!=(const AndConditionExpression& other) const { return !(*this == other); }

 private:
  QSet<ConditionExpression> m_condSet;
};

}  // namespace core
