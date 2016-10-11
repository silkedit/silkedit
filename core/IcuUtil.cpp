#include "IcuUtil.h"
#include "scoped_guard.h"

#include <QDebug>
#include <QVector>

namespace core {
QString IcuUtil::toQString(const UnicodeString& string) {
  int len = string.length();
  QString ret(len, Qt::Uninitialized);
  string.extract(0, len, reinterpret_cast<UChar*>(ret.data()));
  return ret;
}

UnicodeString IcuUtil::toIcuString(const QString& string) {
  return icu::UnicodeString::fromUTF8(string.toUtf8().constData());
}

Locale IcuUtil::icuLocale(const QString& localeStr) {
  return Locale::createCanonical(localeStr.toUtf8().constData());
}

QVector<int> IcuUtil::wordBoundaries(const QString& text, Locale locale) {
  UErrorCode status = U_ZERO_ERROR;
  auto boundary = BreakIterator::createWordInstance(locale, status);
  if (!boundary) {
    qWarning() << "Failed to createWordInstance using locale:" << locale.getName()
               << "use en_US locale instead";
    boundary = BreakIterator::createWordInstance(Locale::getUS(), status);
    if (!boundary) {
      throw std::runtime_error("Failed to createWordInstance using en_US locale");
    }
  }
  Q_ASSERT(boundary);

  scoped_guard guard([=] { delete boundary; });
  boundary->setText(IcuUtil::toIcuString(text));
  int32_t pos = boundary->first();

  QVector<int> positions;
  while (pos != BreakIterator::DONE) {
    positions.append(pos);
    pos = boundary->next();
  }

  return positions;
}

}  // namespace core
