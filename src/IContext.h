#pragma once

#include "macros.h"

class QString;

enum class Operator {
  EQUALS,
  NOT_EQUALS,
};

class IContext {
  DISABLE_COPY_AND_MOVE(IContext)
 public:
  virtual ~IContext() = default;

  virtual bool isSatisfied(Operator op, const QString& operand);

 protected:
  IContext() = default;

 private:
  virtual QString key() = 0;
};
