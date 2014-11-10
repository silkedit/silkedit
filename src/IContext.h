#pragma once

#include <functional>

#include "macros.h"

class QVariant;
class QString;

enum class Operator {
  EQUALS,
};

class IContext {
  DISABLE_COPY_AND_MOVE(IContext)
 public:
  IContext() = default;
  virtual ~IContext() = default;

  virtual bool isSatisfied() = 0;
};

typedef std::function<std::shared_ptr<IContext>(Operator, const QString&)> CREATION_METHOD;

template <typename T>
class IContextBase : public IContext {
  DISABLE_COPY_AND_MOVE(IContextBase)
 public:
  IContextBase<T>(Operator op, const T& operand);
  virtual ~IContextBase() = default;

  bool isSatisfied() override;

 private:
  Operator m_op;
  T m_operand;

  virtual T key() = 0;
};
