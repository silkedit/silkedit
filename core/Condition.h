#pragma once

#include <QObject>

#include "macros.h"

class QString;

namespace core {

class Condition : public QObject {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(Condition)
 public:
  enum Operator {
    EQUALS,
    NOT_EQUALS,
  };
  Q_ENUM(Operator)

  static bool check(const QString &key, Operator op, const QString& operand);

  static QString operatorString(Operator op);

  virtual ~Condition() = default;


public slots:
  virtual bool isSatisfied(Operator op, const QString& operand);

  /**
   * @brief if condition is static or not
   * @return
   */
  virtual bool isStatic() = 0;

 protected:
  Condition() = default;

 private:
  virtual QString key() = 0;
};

}  // namespace core

//Q_DECLARE_METATYPE(core::Condition::Operator)
