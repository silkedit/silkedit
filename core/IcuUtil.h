#pragma once

// both Onigmo and ICU defines typedef UChar but they are different types.
// So IcuUtil exists independent Util class.
#include <unicode/brkiter.h>
#include <QString>

namespace core {

class IcuUtil {
 public:
  static QString toQString(const icu::UnicodeString& string);

  static icu::UnicodeString toIcuString(const QString& string);

  static Locale icuLocale(const QString& localeStr);

  static QVector<int> wordBoundaries(const QString& text, Locale locale = Locale::getJapanese());

 private:
  IcuUtil() = delete;
  ~IcuUtil() = delete;
};

}  // namespace core
