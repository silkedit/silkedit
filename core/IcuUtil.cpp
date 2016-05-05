#include "IcuUtil.h"

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

Locale IcuUtil::icuLocale(const QString &localeStr) {
  return Locale::createCanonical(localeStr.toUtf8().constData());
}

}  // namespace core
