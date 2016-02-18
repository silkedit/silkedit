#pragma once

#include <QTextBlock>
#include <QMetaType>

#include "Wrapper.h"

namespace core {

// Wrapper of QTextBlock
class TextBlock : public Wrapper {
  Q_OBJECT
  Q_CLASSINFO(WRAPPED, "QTextBlock")

 public:
  TextBlock(QTextBlock Block) { m_wrapped = QVariant::fromValue(Block); }
  Q_INVOKABLE TextBlock() { m_wrapped = QVariant::fromValue(QTextBlock()); }
  ~TextBlock() = default;

 public slots:
  QString text() const;
  int position() const;
};

}  // namespace core

Q_DECLARE_METATYPE(core::TextBlock*)
Q_DECLARE_METATYPE(QTextBlock)
