#pragma once

#include "macros.h"

class QString;

namespace core {

enum class Operator {
  EQUALS,
  NOT_EQUALS,
};

class ICondition {
  DISABLE_COPY_AND_MOVE(ICondition)
 public:
  static QString operatorString(Operator op);

  virtual ~ICondition() = default;

  virtual bool isSatisfied(Operator op, const QString& operand);

  /**
   * @brief if condition is static or not
   * @return
   */
  virtual bool isStatic() = 0;

 protected:
  ICondition() = default;

 private:
  virtual QString key() = 0;
};

}  // namespace core
