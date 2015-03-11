#pragma once

#include "IContext.h"
#include "Singleton.h"
#include "macros.h"

class OSContext : public Singleton<OSContext>, public IContext {
  DISABLE_COPY(OSContext)

 public:
  static const QString name;

  ~OSContext() = default;
  DEFAULT_MOVE(OSContext)

 private:
  friend class Singleton<OSContext>;

  OSContext() = default;
  QString key() override;
};
