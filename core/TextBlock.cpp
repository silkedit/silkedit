#include "TextBlock.h"

namespace core {

QString TextBlock::text() const
{
  return m_wrapped.value<QTextBlock>().text();
}

int TextBlock::position() const {
  return m_wrapped.value<QTextBlock>().position();
}

}  // namespace core
