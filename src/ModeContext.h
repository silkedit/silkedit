#pragma once

#include "IContext.h"
#include "ViEngine.h"

class ModeContext : public IContextBase<QString> {
  DISABLE_COPY(ModeContext)
 public:
  ModeContext(ViEngine* viEngine, Operator op, const QString& operand);
  virtual ~ModeContext() = default;
  DEFAULT_MOVE(ModeContext)

 private:
  ViEngine* m_viEngine;

  QString key() override;
};
