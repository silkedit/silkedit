#pragma once

#include <QString>
#include "core/IContext.h"

class PluginContext : public core::IContext {
  DISABLE_COPY(PluginContext)

 public:
  PluginContext(const QString& context);
  ~PluginContext() = default;

  bool isSatisfied(core::Operator op, const QString& operand) override;

 private:
  QString m_key;

  QString key() override;
};
