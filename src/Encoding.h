#pragma once

#include <string>
#include <boost/optional.hpp>
#include <QList>
#include <QString>

#include "macros.h"

class Encoding {
 public:
  static const QList<Encoding> availableEncodings();
  static const Encoding guessEncoding(const QByteArray& bytes);
  static const Encoding defaultEncoding();
  static const boost::optional<Encoding> encodingForName(const QString& name);

  Encoding(const QString& name, const QString& displayName);
  ~Encoding() = default;
  DEFAULT_COPY_AND_MOVE(Encoding)

  QString name() const { return m_name; }
  QString displayName() const { return m_displayName; }
  QTextCodec* codec() const;

  bool operator==(const Encoding& other) const;
  bool operator!=(const Encoding& other) const;

 private:
  // Encoding name
  // See http://www.iana.org/assignments/character-sets/character-sets.xml
  QString m_name;

  // Display name. (e.g., "Japanese (Shift_JIS)")
  QString m_displayName;
};
