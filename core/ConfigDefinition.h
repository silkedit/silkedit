#pragma once

#include <QString>
#include <QVariant>
#include "core/macros.h"

namespace core {

struct ConfigDefinition {
  QString key;
  QString title;
  QString description;
  QVariant defaultValue;

  QVariant::Type type() const { return defaultValue.type(); }
};

}  // namespace core
