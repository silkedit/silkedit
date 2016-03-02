#include <QDebug>

#include "TextOption.h"

namespace core {

int TextOption::flags() const {
  return static_cast<QTextOption::Flags>(m_wrapped.value<QTextOption>().flags());
}

void TextOption::setFlags(int flags) {
  qDebug() << flags;
  auto option = m_wrapped.value<QTextOption>();
  option.setFlags(static_cast<QTextOption::Flags>(flags));
  m_wrapped = QVariant::fromValue(option);
}

}  // namespace core

