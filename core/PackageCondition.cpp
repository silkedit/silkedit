#include <sstream>
#include <QDebug>
#include <QLoggingCategory>

#include "PackageCondition.h"
#include "V8Util.h"

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
using v8::Context;

namespace core {

Persistent<Function> PackageCondition::constructor;

PackageCondition::PackageCondition(v8::Isolate* isolate, v8::Local<v8::Object> object)
    : m_isolate(isolate) {
  m_object.Reset(isolate, object);
}

bool PackageCondition::isSatisfied(const QString &op, const QVariant &operand) {
  v8::Locker locker(m_isolate);
  v8::HandleScope handleScope(m_isolate);

  auto object = Local<Object>::New(m_isolate, m_object);
  MaybeLocal<Value> maybeIsSatisfiedFn = object->Get(
      m_isolate->GetCurrentContext(), v8::String::NewFromUtf8(m_isolate, "isSatisfied"));
  if (maybeIsSatisfiedFn.IsEmpty()) {
    return Condition::isSatisfied(op, operand);
  }

  Local<Value> isSatisfiedValue = maybeIsSatisfiedFn.ToLocalChecked();
  if (!isSatisfiedValue->IsFunction()) {
    return Condition::isSatisfied(op, operand);
  }

  Local<Function> isSatisfiedFn = Local<Function>::Cast(isSatisfiedValue);
  const int argc = 2;
  Local<Value> argv[argc];
  argv[0] = V8Util::toV8Value(m_isolate, op);
  argv[1] = V8Util::toV8Value(m_isolate, operand);

  auto result = V8Util::callJSFunc(m_isolate, isSatisfiedFn, object, argc, argv);

  if (!result.canConvert<bool>()) {
    QLoggingCategory category("silkedit");
    qCCritical(category) << "result is not boolean";
    return false;
  }

  return result.toBool();
}

QVariant PackageCondition::value() {
  auto object = Local<Object>::New(m_isolate, m_object);
  MaybeLocal<Value> maybeValueFn =
      object->Get(m_isolate->GetCurrentContext(), v8::String::NewFromUtf8(m_isolate, "value"));
  if (maybeValueFn.IsEmpty()) {
    throw std::runtime_error("value is empty");
  }

  Local<Value> valueV8Value = maybeValueFn.ToLocalChecked();
  if (!valueV8Value->IsFunction()) {
    throw std::runtime_error("value is not function");
  }

  Local<Function> valueFn = Local<Function>::Cast(valueV8Value);

  TryCatch trycatch(m_isolate);
  MaybeLocal<Value> maybeResult =
      valueFn->Call(m_isolate->GetCurrentContext(), v8::Undefined(m_isolate), 0, nullptr);

  if (trycatch.HasCaught()) {
    MaybeLocal<Value> maybeStackTrace = trycatch.StackTrace(m_isolate->GetCurrentContext());
    Local<Value> exception = trycatch.Exception();
    String::Utf8Value exceptionStr(exception);
    std::stringstream ss;
    ss << "error: " << *exceptionStr;
    if (!maybeStackTrace.IsEmpty()) {
      String::Utf8Value stackTraceStr(maybeStackTrace.ToLocalChecked());
      ss << " stack trace: " << *stackTraceStr;
    }
    throw std::runtime_error(ss.str());
  } else if (maybeResult.IsEmpty()) {
    throw std::runtime_error("maybeResult is empty (but exception is not thrown...)");
  }

  Local<Value> result = maybeResult.ToLocalChecked();
  if (result.IsEmpty()) {
    throw std::runtime_error("result is empty");
  }

  return V8Util::toVariant(m_isolate, result);
}

}  // namespace core
