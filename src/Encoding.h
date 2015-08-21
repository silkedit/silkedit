#pragma once

#include <string>
#include <QList>
#include <QString>

#include "macros.h"

class Encoding {
 public:
  static QList<Encoding> availableEncodings() { return s_availableEncodings; }
  static Encoding guessEncoding(const std::string& text);
  static Encoding defaultEncoding();

  Encoding(const QString& name, const QString& displayName);
  ~Encoding() = default;
  DEFAULT_COPY_AND_MOVE(Encoding)

  QString name() const { return m_name; }
  QString displayName() const { return m_displayName; }
  QTextCodec* codec() const;

 private:
  const static QList<Encoding> s_availableEncodings;

  // Encoding name
  // See http://www.iana.org/assignments/character-sets/character-sets.xml
  QString m_name;

  // Display name. (e.g., "Japanese (Shift_JIS)")
  QString m_displayName;
};
