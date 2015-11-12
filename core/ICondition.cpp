#include <QString>

#include "ICondition.h"

namespace core {

QString ICondition::operatorString(Operator op) {
  switch (op) {
    case Operator::EQUALS:
      return "==";
    case Operator::NOT_EQUALS:
      return "!=";
    default:
      return "";
  }
}

bool ICondition::isSatisfied(Operator op, const QString& operand) {
  switch (op) {
    case Operator::EQUALS:
      return key() == operand;
    case Operator::NOT_EQUALS:
      return key() != operand;
    default:
      return false;
  }
}

}  // namespace core
