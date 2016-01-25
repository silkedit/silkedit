#include <QString>
#include <QDebug>

#include "Condition.h"
#include "OSCondition.h"
#include "PackageCondition.h"
#include "V8Util.h"
#include "atom/node_includes.h"

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
using v8::TryCatch;
using v8::ObjectTemplate;
using v8::Maybe;
using v8::Boolean;

namespace core {

  std::unordered_map<QString, std::unique_ptr<Condition>> Condition::s_conditions;

bool Condition::check(const QString& key, Condition::Operator op, const QString& operand) {
  switch (op) {
    case Operator::EQUALS:
      return key == operand;
    case Operator::NOT_EQUALS:
      return key != operand;
    default:
      return false;
  }
}

QString Condition::operatorString(Operator op) {
  switch (op) {
    case Operator::EQUALS:
      return "==";
    case Operator::NOT_EQUALS:
      return "!=";
    default:
      return "";
  }
}

void core::Condition::Init(v8::Local<v8::Object> exports) {
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
  NODE_SET_METHOD(obj, "check", Check);

  Maybe<bool> result =
      exports->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "Condition"), obj);
  if (result.IsNothing()) {
    throw std::runtime_error("setting exports failed");
  }
}

bool core::Condition::isStatic(const QString& key) {
  if (s_conditions.find(key) == s_conditions.end())
    return false;

  return s_conditions.at(key)->isStatic();
}

bool core::Condition::isSatisfied(const QString& key,
                                  core::Condition::Operator op,
                                  const QString& value) {
  if (s_conditions.find(key) == s_conditions.end()) {
    return false;
  }

  return s_conditions.at(key)->isSatisfied(op, value);
}

void core::Condition::init() {
  s_conditions.clear();
  // register default conditions
  add(OSCondition::name, std::move(std::unique_ptr<Condition>(new OSCondition())));
  add(OnMacCondition::name, std::move(std::unique_ptr<Condition>(new OnMacCondition())));
  add(OnWindowsCondition::name, std::move(std::unique_ptr<Condition>(new OnWindowsCondition())));
}

bool Condition::isSatisfied(Operator op, const QString& operand) {
  try {
    return check(key(), op, operand);
  } catch (const std::exception& e) {
    qWarning() << e.what();
  } catch (...) {
    qWarning() << "unexpected exception";
  }

  return false;
}

void core::Condition::add(const QString& key, std::unique_ptr<core::Condition> condition) {
  s_conditions[key] = std::move(condition);
}

void core::Condition::remove(const QString& key) {
  s_conditions.erase(key);
}

void core::Condition::Remove(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1 || !args[0]->IsString()) {
    V8Util::throwError(isolate, "invalid argument");
    return;
  }

  const auto& key =
      V8Util::toQString(args[0]->ToString(isolate->GetCurrentContext()).ToLocalChecked());
  remove(key);
}

void Condition::Check(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (!V8Util::checkArguments(args, 3, [&] {
        return args[0]->IsString() && args[1]->IsInt32() && args[2]->IsString();
      })) {
    return;
  }

  bool result = check(V8Util::toQString(args[0]->ToString()),
                      static_cast<Condition::Operator>(args[1]->ToInt32()->Value()),
                      V8Util::toQString(args[2]->ToString()));
  args.GetReturnValue().Set(Boolean::New(args.GetIsolate(), result));
}

void core::Condition::Add(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsObject()) {
    V8Util::throwError(isolate, "invalid argument");
    return;
  }

  const auto& key =
      V8Util::toQString(args[0]->ToString(isolate->GetCurrentContext()).ToLocalChecked());
  add(key, std::move(std::unique_ptr<core::Condition>(new PackageCondition(
               isolate, args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()))));
}

}  // namespace core
