#include <QString>

#include "IContext.h"

template <typename T>
IContextBase<T>::IContextBase(Operator op, const T& operand)
    : m_op(op), m_operand(std::move(operand)) {}

template <typename T>
bool IContextBase<T>::isSatisfied() {
  switch (m_op) {
    case Operator::EQUALS:
      return key() == m_operand;
      break;
    default:
      break;
  }
}

template class IContextBase<QString>;
