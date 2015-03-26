#pragma once

#include <QString>
#include "IContext.h"

class PluginContext : public IContext {
  DISABLE_COPY(PluginContext)

 public:
  PluginContext(const QString& context);
  ~PluginContext() = default;

  bool isSatisfied(Operator op, const QString& operand) override;

 private:
  QString m_key;

  QString key() override;
};
