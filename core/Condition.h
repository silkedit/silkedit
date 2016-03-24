#pragma once

#include <QObject>

#include "macros.h"

class QString;

namespace core {

class Condition : public QObject {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(Condition)
 public:
  static QString equalsOperator;
  static QString notEqualsOperator;

  virtual ~Condition() = default;

  virtual bool isSatisfied(const QString& op, const QVariant& operand);

  /**
   * @brief if condition is static or not
   * @return
   */
  virtual bool isStatic() = 0;

 protected:
  Condition() = default;

 private:
  static bool check(const QVariant& value, const QString& op, const QVariant& operand);

  virtual QVariant value() = 0;
};

}  // namespace core
