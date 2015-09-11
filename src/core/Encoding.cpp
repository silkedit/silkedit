#include <uchardet/uchardet.h>
#include <algorithm>
#include <QTextCodec>
#include <QDebug>

#include "core/Encoding.h"

using core::Encoding;

namespace {
const int MAX_LENGTH_TO_READ = 256 * 1024;  // 256KB

QString guessEncodingInternal(const QByteArray& bytes) {
  // Detect character code set
  uchardet_t ucd = uchardet_new();
  if (uchardet_handle_data(ucd, bytes, std::min(bytes.size(), MAX_LENGTH_TO_READ)) != 0) {
    return "";
  }

  uchardet_data_end(ucd);
  const char* charSetName = uchardet_get_charset(ucd);
  QString csn = charSetName;
  qDebug() << "Detected character set: " << csn;
  uchardet_delete(ucd);
  return csn;
}

const QString DEFAULT_ENCODING_NAME = "UTF-8";

const Encoding DEFAULT_ENCODING = Encoding("UTF-8", QObject::tr("UTF-8"));
}

namespace core {

const QList<Encoding> Encoding::availableEncodings() {
  static const QList<Encoding> s_availableEncodings =
      QList<Encoding>{DEFAULT_ENCODING,
                      // Japanese
                      Encoding("Shift_JIS", QObject::tr("Japanese (Shift_JIS)")),
                      Encoding("EUC-JP", QObject::tr("Japanese (EUC-JP)")),
                      Encoding("ISO-2022-JP", QObject::tr("Japanese (ISO-2022-JP)")),
                      // Others
                      Encoding("ISO 8859-6", QObject::tr("Arabic (ISO 8859-6)")),
                      Encoding("Windows-1256", QObject::tr("Arabic (Windows-1256)")),
                      Encoding("ISO 8859-4", QObject::tr("Baltic (ISO 8859-4)")),
                      Encoding("ISO 8859-13", QObject::tr("Baltic (ISO 8859-13)")),
                      Encoding("Windows-1257", QObject::tr("Baltic (Windows-1257)")),
                      Encoding("ISO 8859-14", QObject::tr("Celtic (ISO 8859-14)")),
                      Encoding("ISO 8859-2", QObject::tr("Central European (ISO 8859-2)")),
                      Encoding("Windows-1250", QObject::tr("Central European (Windows-1250)")),
                      Encoding("GB18030", QObject::tr("Chinese Simplified (GB18030)")),
                      Encoding("Big5", QObject::tr("Chinese Traditional (Big5)")),
                      Encoding("Big5-HKSCS", QObject::tr("Chinese Traditional (Big5-HKSCS)")),
                      Encoding("ISO 8859-5", QObject::tr("Cyrillic (ISO 8859-5)")),
                      Encoding("Windows-1251", QObject::tr("Cyrillic (Windows-1251)")),
                      Encoding("KOI8-R", QObject::tr("Cyrillic (KOI8-R)")),
                      Encoding("KOI8-U", QObject::tr("Cyrillic (KOI8-U)")),
                      Encoding("ISO 8859-7", QObject::tr("Greek (ISO 8859-7)")),
                      Encoding("Windows-1253", QObject::tr("Greek (Windows-1253)")),
                      Encoding("ISO 8859-8", QObject::tr("Hebrew (ISO 8859-8)")),
                      Encoding("Windows-1255", QObject::tr("Hebrew (Windows-1255)")),
                      Encoding("EUC-KR", QObject::tr("Korean (EUC-KR)")),
                      Encoding("ISO 8859-10", QObject::tr("Nordic (ISO 8859-10)")),
                      Encoding("ISO 8859-16", QObject::tr("Romanian (ISO 8859-16)")),
                      Encoding("ISO 8859-3", QObject::tr("South European (ISO 8859-3)")),
                      Encoding("IBM 874", QObject::tr("Thai (IBM 874)")),
                      Encoding("ISO 8859-9", QObject::tr("Turkish (ISO 8859-9)")),
                      Encoding("Windows-1254", QObject::tr("Turkish (Windows-1254)")),
                      Encoding("UTF-16BE", QObject::tr("UTF-16BE")),
                      Encoding("UTF-16LE", QObject::tr("UTF-16LE")),
                      Encoding("UTF-32BE", QObject::tr("UTF-32BE")),
                      Encoding("UTF-32LE", QObject::tr("UTF-32LE")),
                      Encoding("Windows-1258", QObject::tr("Vietnamese (Windows-1258)")),
                      Encoding("ISO 8859-1", QObject::tr("Western (ISO 8859-1)")),
                      Encoding("ISO 8859-15", QObject::tr("Western (ISO 8859-15)")),
                      Encoding("Windows-1252", QObject::tr("Western (Windows-1252)")),
                      Encoding("Macintosh", QObject::tr("Western (Macintosh)"))};

  return s_availableEncodings;
}

const Encoding Encoding::guessEncoding(const QByteArray& bytes) {
  QString encName = guessEncodingInternal(bytes);
  auto encodingIter =
      std::find_if(availableEncodings().begin(), availableEncodings().end(),
                   [encName](const QList<Encoding>::value_type& p) { return p.name() == encName; });
  if (encodingIter != availableEncodings().end()) {
    return *encodingIter;
  }
  return DEFAULT_ENCODING;
}

const Encoding Encoding::defaultEncoding() {
  return DEFAULT_ENCODING;
}

const boost::optional<Encoding> Encoding::encodingForName(const QString& name) {
  auto encodingIter =
      std::find_if(availableEncodings().begin(), availableEncodings().end(),
                   [name](const QList<Encoding>::value_type& p) { return p.name() == name; });
  if (encodingIter != availableEncodings().end()) {
    return *encodingIter;
  }
  return boost::none;
}

Encoding::Encoding(const QString& name, const QString& displayName)
    : m_name(name), m_displayName(displayName) {
}

QTextCodec* Encoding::codec() const {
  return QTextCodec::codecForName(name().toUtf8().constData());
}

bool Encoding::operator==(const Encoding& other) const {
  return this->name() == other.name();
}

bool Encoding::operator!=(const Encoding& other) const {
  return !(*this == other);
}

}  // namespace core
