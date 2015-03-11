#include <QString>

#include "IContext.h"

bool IContext::isSatisfied(Operator op, const QString& operand) {
  switch (op) {
    case Operator::EQUALS:
      return key() == operand;
    case Operator::NOT_EQUALS:
      return key() != operand;
    default:
      return false;
  }
}
