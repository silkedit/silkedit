#pragma once

#include <QString>

#include "core/macros.h"

namespace core {

class LineSeparator {
 public:
  ~LineSeparator() = default;
  DEFAULT_COPY_AND_MOVE(LineSeparator)

  static const LineSeparator Windows;
  static const LineSeparator Unix;
  static const LineSeparator ClassicMac;

  static const LineSeparator guess(const QString& text);

  QString separatorStr() const { return m_separator; }
  QString displayName() const { return m_displayName; }
  static const LineSeparator defaultLineSeparator();

  bool operator==(const LineSeparator& other) const;
  bool operator!=(const LineSeparator& other) const;

 private:
  LineSeparator(const QString& separatorStr, const QString& displayName);

  const QString m_separator;
  const QString m_displayName;
};

}  // namespace core
