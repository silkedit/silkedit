#include <QString>
#include <QDebug>

#include "Condition.h"

namespace core {

bool Condition::check(const QString& key, Condition::Operator op, const QString& operand) {
  switch (op) {
    case Operator::EQUALS:
      return key == operand;
    case Operator::NOT_EQUALS:
      return key != operand;
    default:
      return false;
  }
}

QString Condition::operatorString(Operator op) {
  switch (op) {
    case Operator::EQUALS:
      return "==";
    case Operator::NOT_EQUALS:
      return "!=";
    default:
      return "";
  }
}

bool Condition::isSatisfied(Operator op, const QString& operand) {
  try {
    return check(key(), op, operand);
  } catch (const std::exception& e) {
    qWarning() << e.what();
  } catch (...) {
    qWarning() << "unexpected exception";
  }

  return false;
}

}  // namespace core
