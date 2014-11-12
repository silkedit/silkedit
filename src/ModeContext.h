#pragma once

#include "IContext.h"
#include "ViEngine.h"

class ModeContext : public IContextBase<QString> {
  DISABLE_COPY(ModeContext)
 public:
  static const QString name;

  ModeContext(ViEngine* viEngine, Operator op, const QString& operand);
  ~ModeContext() = default;
  DEFAULT_MOVE(ModeContext)

 private:
  ViEngine* m_viEngine;

  QString key() override;
};

class ModeContextCreator : public IContextCreator {
  DISABLE_COPY(ModeContextCreator)
 public:
  ModeContextCreator(ViEngine* viEngine);
  ~ModeContextCreator() = default;
  DEFAULT_MOVE(ModeContextCreator)

  std::shared_ptr<IContext> create(Operator op, const QString& operand) override;

 private:
  ViEngine* m_viEngine;
};
