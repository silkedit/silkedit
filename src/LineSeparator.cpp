#include "LineSeparator.h"

const LineSeparator LineSeparator::Windows = LineSeparator("\r\n", "CRLF - Windows(\\r\\n)");
const LineSeparator LineSeparator::Unix = LineSeparator("\n", "LF - Unix and OS X(\\n)");
const LineSeparator LineSeparator::ClassicMac = LineSeparator("\r", "CR - Classic Mac(\\r)");

LineSeparator LineSeparator::defaultLineSeparator()
{
#ifdef Q_OS_WIN
  return Windows;
#else
  return Unix;
#endif
}

LineSeparator::LineSeparator(const QString& separator, const QString& displayName)
    : m_separator(separator), m_displayName(displayName) {
}
