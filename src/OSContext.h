#pragma once

#include "core/IContext.h"
#include "macros.h"

class OSContext : public core::IContext {
  DISABLE_COPY(OSContext)

 public:
  static const QString name;

  OSContext() = default;
  ~OSContext() = default;
  DEFAULT_MOVE(OSContext)

 private:
  QString key() override;
};
