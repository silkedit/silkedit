#pragma once

#include <QUrl>
#include <QMetaType>

#include "Wrapper.h"

namespace core {

class Url : public Wrapper {
  Q_OBJECT
  Q_CLASSINFO(WRAPPED, "QUrl")

 public:
  // redefined
  enum ParsingMode { TolerantMode, StrictMode, DecodedMode };
  Q_ENUM(ParsingMode)

  Q_INVOKABLE Url() { m_wrapped = QVariant::fromValue(QUrl()); }

  Q_INVOKABLE Url(const QString& url, ParsingMode parsingMode = TolerantMode) {
    m_wrapped = QVariant::fromValue(QUrl(url, static_cast<QUrl::ParsingMode>(parsingMode)));
  }

  ~Url() = default;
};

}  // namespace core

Q_DECLARE_METATYPE(core::Url*)
Q_DECLARE_METATYPE(QUrl)
