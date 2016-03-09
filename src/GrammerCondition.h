#pragma once

#include <QVariant>

#include "core/Condition.h"
#include "core/macros.h"

class GrammerCondition : public core::Condition {
  DISABLE_COPY(GrammerCondition)

 public:
  static const QString name;

  GrammerCondition() = default;
  ~GrammerCondition() = default;
  DEFAULT_MOVE(GrammerCondition)

  bool isStatic() override { return false; }

 private:
  QVariant keyValue() override;
};
