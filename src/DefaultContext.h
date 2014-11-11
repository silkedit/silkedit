#pragma once

#include "macros.h"
#include "IContext.h"

class DefaultContext : public IContext {
  DISABLE_COPY(DefaultContext)
 public:
  DefaultContext() = default;
  ~DefaultContext() = default;
  DEFAULT_MOVE(DefaultContext)

  bool isSatisfied() override { return true; }
};
