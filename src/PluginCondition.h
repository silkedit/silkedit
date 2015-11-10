#pragma once

#include <QString>
#include "core/ICondition.h"

class PluginCondition : public core::ICondition {
  DISABLE_COPY(PluginCondition)

 public:
  PluginCondition(const QString& key);
  ~PluginCondition() = default;

  bool isSatisfied(core::Operator op, const QString& operand) override;
  bool isStatic() override { return false; }

 private:
  QString m_key;

  QString key() override;
};
