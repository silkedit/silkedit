#include <QRegularExpression>

#include "LineSeparator.h"

namespace core {

const LineSeparator LineSeparator::Windows =
    LineSeparator("\r\n", "CRLF - Windows(\\r\\n)", "CRLF");
const LineSeparator LineSeparator::Unix = LineSeparator("\n", "LF - Unix and OS X(\\n)", "LF");
const LineSeparator LineSeparator::ClassicMac = LineSeparator("\r", "CR - Classic Mac(\\r)", "CR");

const LineSeparator LineSeparator::guess(const QString& text) {
  int index = text.indexOf(QRegularExpression("\r|\n"));
  if (index < 0) {
    return LineSeparator::defaultLineSeparator();
  } else {
    QChar firstLineSeparatorChar = text.at(index);
    if (firstLineSeparatorChar == '\r') {
      // \r is the last character
      if (index >= text.size() - 1) {
        return ClassicMac;
      }

      if (text.at(index + 1) == '\n') {
        return Windows;
      } else {
        return ClassicMac;
      }
    } else if (firstLineSeparatorChar == '\n') {
      return Unix;
    } else {
      Q_ASSERT(false);
      return LineSeparator::defaultLineSeparator();
    }
  }
}

const LineSeparator LineSeparator::defaultLineSeparator() {
#ifdef Q_OS_WIN
  return Windows;
#else
  return Unix;
#endif
}

bool LineSeparator::operator==(const LineSeparator& other) const {
  return this->separatorStr() == other.separatorStr();
}

bool LineSeparator::operator!=(const LineSeparator& other) const {
  return !(*this == other);
}

LineSeparator::LineSeparator(const QString& separator,
                             const QString& displayName,
                             const QString& shortDisplayName)
    : m_separator(separator), m_displayName(displayName), m_shortDisplayName(shortDisplayName) {
}

}  // namespace core
