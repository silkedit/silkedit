#include <libguess/libguess.h>
#include <algorithm>
#include <QTextCodec>

#include "Encoding.h"

namespace {
const char* guessEncodingInternal(const std::string& text) {
  /*
   - libguess_determine_encoding(const char *inbuf, int length, const char *region);

    This detects a character set.  Returns an appropriate charset name
    that can be passed to iconv_open().  Region is the name of the language
    or region that the data is related to, e.g. 'Baltic' for the Baltic states,
    or 'Japanese' for Japan.
  */
  const int length = 256 * 1024;  // 256KB
  return libguess_determine_encoding(text.c_str(), length, "Japanese");
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

Encoding Encoding::guessEncoding(const std::string& text) {
  const char* encName = guessEncodingInternal(text);
  auto encodingIter =
      std::find_if(s_availableEncodings.begin(),
                   s_availableEncodings.end(),
                   [encName](const QList<Encoding>::value_type& p) { return p.name() == encName; });
  if (encodingIter != s_availableEncodings.end()) {
    return *encodingIter;
  }
  return DEFAULT_ENCODING;
}

Encoding Encoding::defaultEncoding()
{
  return DEFAULT_ENCODING;
}

Encoding::Encoding(const QString& name, const QString& displayName)
    : m_name(name), m_displayName(displayName) {
}

QTextCodec* Encoding::codec() const {
  return QTextCodec::codecForName(name().toUtf8().constData());
}
