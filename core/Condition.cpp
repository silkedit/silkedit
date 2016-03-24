#include <QString>
#include <QDebug>

#include "Condition.h"

namespace core {

QString Condition::equalsOperator = QStringLiteral("==");
QString Condition::notEqualsOperator = QStringLiteral("!=");

bool Condition::check(const QVariant& value, const QString& op, const QVariant& operand) {
  if (op == equalsOperator) {
    return value == operand;
  } else if (op == notEqualsOperator) {
    return value != operand;
  } else {
    qWarning() << op << "is not supported";
    return false;
  }
}

bool Condition::isSatisfied(const QString& op, const QVariant& operand) {
  try {
    return check(value(), op, operand);
  } catch (const std::exception& e) {
    qWarning() << e.what();
  } catch (...) {
    qWarning() << "unexpected exception";
  }

  return false;
}

}  // namespace core
