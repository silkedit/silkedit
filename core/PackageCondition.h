#pragma once

#include <v8.h>
#include <QString>
#include "Condition.h"

namespace core {

class PackageCondition : public Condition {
  Q_OBJECT
  DISABLE_COPY(PackageCondition)

 public:
  PackageCondition(v8::Isolate* isolate, v8::Local<v8::Object> object);
  ~PackageCondition() = default;

  bool isSatisfied(const QString& op, const QVariant& operand) override;
  bool isStatic() override { return false; }

 private:
  static v8::Persistent<v8::Function> constructor;

  v8::UniquePersistent<v8::Object> m_object;
  v8::Isolate* m_isolate;

  QVariant value() override;
};

}  // namespace core
