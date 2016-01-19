#pragma once

#include <QVariant>

namespace core {

struct QVariantArgument {
  operator QGenericArgument() const {
    if (value.isValid()) {
      return QGenericArgument(value.typeName(), value.constData());
    } else {
      return QGenericArgument();
    }
  }

  QVariant value;
};

}
