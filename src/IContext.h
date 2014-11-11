#pragma once

#include "macros.h"

class QVariant;
class QString;

enum class Operator {
  EQUALS,
};

class IContext {
  DISABLE_COPY_AND_MOVE(IContext)
 public:
  virtual ~IContext() = default;

  virtual bool isSatisfied() = 0;

 protected:
  IContext() = default;
};

class IContextCreator {
  DISABLE_COPY_AND_MOVE(IContextCreator)
 public:
  virtual ~IContextCreator() = default;

  virtual std::shared_ptr<IContext> create(Operator op, const QString& operand) = 0;

 protected:
  IContextCreator() = default;
};

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
