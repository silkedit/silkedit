﻿#pragma once

#include "IContext.h"
#include "macros.h"

namespace core {

class OSContext : public IContext {
  DISABLE_COPY(OSContext)

 public:
  static const QString name;

  OSContext() = default;
  ~OSContext() = default;
  DEFAULT_MOVE(OSContext)

  bool isStatic() override { return true; }

 private:
  QString key() override;
};

}  // namespace core
