#pragma once

#include "ICondition.h"
#include "macros.h"

namespace core {

class OSCondition : public ICondition {
  DISABLE_COPY(OSCondition)

 public:
  static const QString name;

  OSCondition() = default;
  ~OSCondition() = default;
  DEFAULT_MOVE(OSCondition)

  bool isStatic() override { return true; }

 private:
  QString key() override;
};

}  // namespace core
