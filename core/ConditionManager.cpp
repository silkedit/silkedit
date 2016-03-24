#include "ConditionManager.h"
#include "V8Util.h"
#include "atom/node_includes.h"
#include "OSCondition.h"
#include "PackageCondition.h"

using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;
using v8::Exception;
using v8::MaybeLocal;
using v8::ObjectTemplate;
using v8::Maybe;
using v8::Boolean;

namespace core {

void ConditionManager::Init(v8::Local<v8::Object> exports) {
  Isolate* isolate = exports->GetIsolate();
  Local<ObjectTemplate> objTempl = ObjectTemplate::New(isolate);
  objTempl->SetInternalFieldCount(1);
  MaybeLocal<Object> maybeObj = objTempl->NewInstance(isolate->GetCurrentContext());
  if (maybeObj.IsEmpty()) {
    throw std::runtime_error("Failed to create ConditionManager");
  }

  Local<Object> obj = maybeObj.ToLocalChecked();

  NODE_SET_METHOD(obj, "add", Add);
  NODE_SET_METHOD(obj, "remove", Remove);

  Maybe<bool> result = exports->Set(
      isolate->GetCurrentContext(),
      String::NewFromUtf8(isolate, "ConditionManager"), obj);
  if (result.IsNothing()) {
    throw std::runtime_error("setting exports failed");
  }
}

void ConditionManager::Remove(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1 || !args[0]->IsString()) {
    V8Util::throwError(isolate, "invalid argument");
    return;
  }

  const auto& key =
      V8Util::toQString(args[0]->ToString(isolate->GetCurrentContext()).ToLocalChecked());
  singleton().remove(key);
}

void ConditionManager::Add(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsObject()) {
    V8Util::throwError(isolate, "invalid argument");
    return;
  }

  const auto& key =
      V8Util::toQString(args[0]->ToString(isolate->GetCurrentContext()).ToLocalChecked());
  singleton().add(key,
                  std::move(std::unique_ptr<core::Condition>(new PackageCondition(
                      isolate, args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()))));
}

bool ConditionManager::isStatic(const QString& key) {
  if (m_conditions.find(key) == m_conditions.end())
    return false;

  return m_conditions.at(key)->isStatic();
}

bool ConditionManager::isSatisfied(const QString& key,
                                   const QString &op,
                                   const QVariant& operand) {
  if (m_conditions.find(key) == m_conditions.end()) {
    return false;
  }

  return m_conditions.at(key)->isSatisfied(op, operand);
}

void ConditionManager::init() {
  m_conditions.clear();
  // register default conditions
  add(OSCondition::name, std::move(std::unique_ptr<Condition>(new OSCondition())));
  add(OnMacCondition::name, std::move(std::unique_ptr<Condition>(new OnMacCondition())));
  add(OnWindowsCondition::name, std::move(std::unique_ptr<Condition>(new OnWindowsCondition())));
}

void ConditionManager::add(const QString& key, std::unique_ptr<core::Condition> condition) {
  m_conditions[key] = std::move(condition);
}

void ConditionManager::remove(const QString& key) {
  m_conditions.erase(key);
}

}  // namespace core
