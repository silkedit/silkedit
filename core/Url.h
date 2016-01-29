#pragma once

#include <QUrl>
#include <QMetaType>

#include "Wrapper.h"

namespace core {

class Url : public Wrapper {
  Q_OBJECT
  Q_CLASSINFO(WRAPPED, "QUrl")

 public:
  Q_INVOKABLE Url() { m_wrapped = QVariant::fromValue(QUrl()); }

  Q_INVOKABLE Url(const QString& url, QUrl::ParsingMode parsingMode = QUrl::TolerantMode) {
    m_wrapped = QVariant::fromValue(QUrl(url, parsingMode));
  }

  ~Url() = default;
};

}  // namespace core

Q_DECLARE_METATYPE(core::Url*)
Q_DECLARE_METATYPE(QUrl)
