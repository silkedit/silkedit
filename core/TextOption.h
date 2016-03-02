#pragma once

#include <QTextOption>
#include <QMetaType>

#include "Wrapper.h"

namespace core {

// Wrapper of QTextOption
class TextOption : public Wrapper {
  Q_OBJECT
  Q_CLASSINFO(WRAPPED, "QTextOption")

 public:
  enum Flag {
    ShowTabsAndSpaces = 0x1,
    ShowLineAndParagraphSeparators = 0x2,
    AddSpaceForLineAndParagraphSeparators = 0x4,
    SuppressColors = 0x8,
    IncludeTrailingSpaces = 0x80000000
  };
  Q_ENUM(Flag)

  TextOption(QTextOption option) { m_wrapped = QVariant::fromValue(option); }
  Q_INVOKABLE TextOption() { m_wrapped = QVariant::fromValue(QTextOption()); }
  ~TextOption() = default;

 public slots:
  int flags() const;
  void setFlags(int flags);
};

}  // namespace core

Q_DECLARE_METATYPE(core::TextOption*)
Q_DECLARE_METATYPE(QTextOption)
