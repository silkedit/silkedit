#include <uchardet/uchardet.h>
#include <algorithm>
#include <QTextCodec>
#include <QDebug>

#include "Encoding.h"

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

const Encoding DEFAULT_ENCODING = Encoding("UTF-8", "UTF-8");
}

// todo: create a map to get an encoding by a name

const QList<Encoding> Encoding::s_availableEncodings =
    QList<Encoding>{DEFAULT_ENCODING,
                    Encoding("Shift_JIS", "Japanese (Shift_JIS)"),
                    Encoding("EUC-JP", "Japanese (EUC-JP)"),
                    Encoding("ISO-2022-JP", "Japanese (ISO-2022-JP)")};

Encoding Encoding::guessEncoding(const QByteArray& bytes) {
  QString encName = guessEncodingInternal(bytes);
  auto encodingIter =
      std::find_if(s_availableEncodings.begin(),
                   s_availableEncodings.end(),
                   [encName](const QList<Encoding>::value_type& p) { return p.name() == encName; });
  if (encodingIter != s_availableEncodings.end()) {
    return *encodingIter;
  }
  return DEFAULT_ENCODING;
}

Encoding Encoding::defaultEncoding() {
  return DEFAULT_ENCODING;
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
