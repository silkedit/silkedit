#pragma once

#include <v8.h>
#include <QObject>
#include <unordered_map>
#include <memory>

#include "Condition.h"
#include "macros.h"
#include "Singleton.h"
#include "stlSpecialization.h"

namespace core {

class ConditionManager : public QObject, public Singleton<ConditionManager> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(ConditionManager)

 public:
  static void Init(v8::Local<v8::Object> exports);

  ~ConditionManager() = default;

  void init();
  bool isStatic(const QString& key);
  bool isSatisfied(const QString& key, const QString& op, const QVariant& operand);
  void add(const QString& key, std::unique_ptr<Condition> condition);
  void remove(const QString& key);

 private:
  static void Add(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Remove(const v8::FunctionCallbackInfo<v8::Value>& args);

  friend class Singleton<ConditionManager>;
  ConditionManager() = default;
  std::unordered_map<QString, std::unique_ptr<Condition>> m_conditions;
};

}  // namespace core
