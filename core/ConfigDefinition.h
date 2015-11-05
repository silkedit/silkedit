#pragma once

#include <QString>
#include <QVariant>

namespace core {

struct ConfigDefinition {
  QString key;
  QString title;
  QString description;
  QVariant defaultValue;

  QVariant::Type type() const { return defaultValue.type(); }
};

}  // namespace core
