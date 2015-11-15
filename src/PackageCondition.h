#pragma once

#include <QString>
#include "core/ICondition.h"

class PackageCondition : public core::ICondition {
  DISABLE_COPY(PackageCondition)

 public:
  PackageCondition(const QString& key);
  ~PackageCondition() = default;

  bool isSatisfied(core::Operator op, const QString& operand) override;
  bool isStatic() override { return false; }

 private:
  QString m_key;

  QString key() override;
};
