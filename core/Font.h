#pragma once

#include <QFont>
#include <QMetaType>

#include "Wrapper.h"

namespace core {

class Font : public Wrapper {
  Q_OBJECT
  Q_CLASSINFO(WRAPPED, "QFont")

 public:
  Q_INVOKABLE Font(const QString& family,
                   int pointSize = -1,
                   int weight = -1,
                   bool italic = false) {
    m_wrapped = QVariant::fromValue(QFont(family, pointSize, weight, italic));
  }
  ~Font() = default;
};

}  // namespace core

Q_DECLARE_METATYPE(core::Font*)
Q_DECLARE_METATYPE(QFont)
