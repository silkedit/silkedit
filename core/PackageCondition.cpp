#include <sstream>
#include <QDebug>

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

PackageCondition::PackageCondition(v8::Isolate *isolate, v8::Local<v8::Object> object) : m_isolate(isolate) {
  m_object.Reset(isolate, object);
}

bool PackageCondition::isSatisfied(Operator op, const QString& operand) {
  v8::Locker locker(m_isolate);
  v8::HandleScope handleScope(m_isolate);

  auto object = Local<Object>::New(m_isolate, m_object);
  MaybeLocal<Value> maybeIsSatisfiedFn =
      object->Get(m_isolate->GetCurrentContext(), v8::String::NewFromUtf8(m_isolate, "isSatisfied"));
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
  argv[0] = v8::Int32::New(m_isolate, static_cast<int>(op));
  argv[1] = V8Util::toV8String(m_isolate, operand);

  TryCatch trycatch(m_isolate);
  MaybeLocal<Value> maybeResult =
      isSatisfiedFn->Call(m_isolate->GetCurrentContext(), v8::Undefined(m_isolate), argc, argv);

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
    qWarning() << ss.str().c_str();
    return false;
  } else if (maybeResult.IsEmpty()) {
    qWarning() << "maybeResult is empty (but exception is not thrown...)";
    return false;
  }

  Local<Value> result = maybeResult.ToLocalChecked();
  if (!result->IsBoolean()) {
    qWarning() << "result is not boolean";
    return false;
  }

  return result->ToBoolean()->Value();
}

QString PackageCondition::key() {
  auto object = Local<Object>::New(m_isolate, m_object);
  MaybeLocal<Value> maybeKeyFn =
      object->Get(m_isolate->GetCurrentContext(), v8::String::NewFromUtf8(m_isolate, "key"));
  if (maybeKeyFn.IsEmpty()) {
    throw std::runtime_error("key is empty");
  }

  Local<Value> keyValue = maybeKeyFn.ToLocalChecked();
  if (!keyValue->IsFunction()) {
    throw std::runtime_error("key is not function");
  }

  Local<Function> keyFn = Local<Function>::Cast(keyValue);

  TryCatch trycatch(m_isolate);
  MaybeLocal<Value> maybeResult =
      keyFn->Call(m_isolate->GetCurrentContext(), v8::Undefined(m_isolate), 0, nullptr);

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
  if (!result->IsString()) {
    throw std::runtime_error("result is not string");
  }

  return V8Util::toQString(result->ToString());
}

}  // namespace core
