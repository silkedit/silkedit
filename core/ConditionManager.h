#pragma once

#include <v8.h>
#include <unordered_map>
#include <memory>
#include <QString>

#include "macros.h"
#include "Condition.h"
#include "stlSpecialization.h"
#include "ConditionExpression.h"
#include "Singleton.h"

namespace core {

class ConditionManager : public Singleton<ConditionManager> {
  DISABLE_COPY_AND_MOVE(ConditionManager)

 public:
  static void Init(v8::Local<v8::Object> exports);

  ~ConditionManager() = default;

  void init();
  bool isStatic(const QString& key);
  bool isSatisfied(const QString& key, core::Condition::Operator op, const QString& value);

 private:
  static void addPackageCondition(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void removePackageCondition(const v8::FunctionCallbackInfo<v8::Value>& args);

  std::unordered_map<QString, std::unique_ptr<Condition>> m_conditions;

  friend class Singleton<ConditionManager>;
  ConditionManager() = default;

  void add(const QString& key, std::unique_ptr<Condition> condition);
  void remove(const QString& key);
};

}  // namespace core
