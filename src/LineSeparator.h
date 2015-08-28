#pragma once

#include <QString>

#include "macros.h"

class LineSeparator {

 public:
  DEFAULT_COPY_AND_MOVE(LineSeparator)

  static const LineSeparator Windows;
  static const LineSeparator Unix;
  static const LineSeparator ClassicMac;

  QString separator() const { return m_separator; }
  QString displayName() const { return m_displayName; }
  LineSeparator defaultLineSeparator();

 private:
  LineSeparator(const QString& separator, const QString& displayName);
  ~LineSeparator() = default;

  const QString m_separator;
  const QString m_displayName;
};
