#pragma once

#include "IContext.h"
#include "ViEngine.h"
#include "Singleton.h"

class ModeContext : public Singleton<ModeContext>, public IContext {
  DISABLE_COPY_AND_MOVE(ModeContext)

 public:
  static const QString name;

  void init(ViEngine* viEngine) { m_viEngine = viEngine; }
  bool isSatisfied(Operator op, const QString& operand) override;

  ~ModeContext() = default;

 private:
  friend class Singleton<ModeContext>;

  ViEngine* m_viEngine;

  ModeContext() = default;
  QString key() override;
};
