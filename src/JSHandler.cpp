#include <node.h>
#include <sstream>

#include "JSHandler.h"
#include "ObjectStore.h"
#include "core/CommandArgument.h"
#include "core/v8adapter.h"
#include "JSObjectHelper.h"

using v8::String;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Exception;
using v8::Value;
using v8::Object;
using v8::Function;
using v8::Persistent;
using v8::HandleScope;
using v8::TryCatch;
using v8::Boolean;
using v8::Array;
using v8::External;
using v8::Symbol;

v8::Persistent<v8::Object> JSHandler::s_jsHandler;

namespace {
constexpr int MAX_ARGS_COUNT_FOR_SIGNAL = MAX_ARGS_COUNT + 1;
}

void JSHandler::init(v8::Local<v8::Object> jsHandler) {
  s_jsHandler.Reset(Isolate::GetCurrent(), jsHandler);
}

QVariant JSHandler::callFunc(const QString& funcName, QVariantList args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  if (args.size() >= MAX_ARGS_COUNT) {
    qWarning() << "max # of args is " << MAX_ARGS_COUNT << ". Exceeded arguments will be dropped";
  }

  Local<Value> argv[MAX_ARGS_COUNT];
  int argc = qMin(args.size(), MAX_ARGS_COUNT);
  for (int i = 0; i < argc; i++) {
    argv[i] = JSObjectHelper::toV8Value(args[i], isolate);
  }

  MaybeLocal<Value> maybeFnValue =
      s_jsHandler.Get(isolate)->Get(isolate->GetCurrentContext(), toV8String(funcName));
  if (maybeFnValue.IsEmpty()) {
    qWarning() << funcName << "not found";
    return QVariant();
  }

  Local<Value> fnValue = maybeFnValue.ToLocalChecked();
  if (!fnValue->IsFunction()) {
    qWarning() << funcName << "is not function";
    return QVariant();
  }
  Local<Function> fn = Local<Function>::Cast(fnValue);

  TryCatch trycatch(isolate);
  MaybeLocal<Value> maybeResult =
      fn->Call(isolate->GetCurrentContext(), s_jsHandler.Get(isolate), argc, argv);
  if (trycatch.HasCaught()) {
    MaybeLocal<Value> maybeStackTrace = trycatch.StackTrace(isolate->GetCurrentContext());
    Local<Value> exception = trycatch.Exception();
    String::Utf8Value exceptionStr(exception);
    std::stringstream ss;
    ss << "error: " << *exceptionStr;
    if (!maybeStackTrace.IsEmpty()) {
      String::Utf8Value stackTraceStr(maybeStackTrace.ToLocalChecked());
      ss << " stack trace: " << *stackTraceStr;
    }
    qWarning() << ss.str().c_str();
    return QVariant();
  } else if (maybeResult.IsEmpty()) {
    qWarning() << "maybeResult is empty (but exception is not thrown...)";
    return QVariant();
  }

  return JSObjectHelper::toVariant(maybeResult.ToLocalChecked());
}

void JSHandler::inheritsQtEventEmitter(Local<v8::Value> proto) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Local<Value> argv[1];
  argv[0] = proto;

  MaybeLocal<Value> maybeFnValue = s_jsHandler.Get(isolate)->Get(
      isolate->GetCurrentContext(),
      String::NewFromUtf8(isolate, "inheritsQtEventEmitter", v8::NewStringType::kNormal)
          .ToLocalChecked());
  if (maybeFnValue.IsEmpty()) {
    qWarning() << "inheritsQtEventEmitter not found";
    return;
  }

  Local<Value> fnValue = maybeFnValue.ToLocalChecked();
  if (!fnValue->IsFunction()) {
    qWarning() << "inheritsQtEventEmitter is not function";
    return;
  }

  Local<Function> fn = Local<Function>::Cast(fnValue);

  TryCatch trycatch(isolate);
  MaybeLocal<Value> result =
      fn->Call(isolate->GetCurrentContext(), s_jsHandler.Get(isolate), 1, argv);
  if (trycatch.HasCaught()) {
    MaybeLocal<Value> maybeStackTrace = trycatch.StackTrace(isolate->GetCurrentContext());
    Local<Value> exception = trycatch.Exception();
    String::Utf8Value exceptionStr(exception);
    std::stringstream ss;
    ss << "error: " << *exceptionStr;
    if (!maybeStackTrace.IsEmpty()) {
      String::Utf8Value stackTraceStr(maybeStackTrace.ToLocalChecked());
      ss << " stack trace: " << *stackTraceStr;
    }
    qWarning() << ss.str().c_str();
  }

  if (result.IsEmpty()) {
    qWarning() << "result is empty";
  }
}

void JSHandler::emitSignal(QObject* obj, const QString& signal, QVariantList args) {
  qDebug() << "emitSignal. " << signal << "args:" << args;
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  if (args.size() >= MAX_ARGS_COUNT) {
    qWarning() << "max # of args is " << MAX_ARGS_COUNT << ". Exceeded arguments will be dropped";
  }

  if (const auto& jsObj = ObjectStore::singleton().find(obj, isolate)) {
    MaybeLocal<Value> maybeEmitValue = (*jsObj)->Get(
        isolate->GetCurrentContext(),
        String::NewFromUtf8(isolate, "emit", v8::NewStringType::kNormal).ToLocalChecked());
    if (maybeEmitValue.IsEmpty()) {
      qWarning() << "emit method not found";
      return;
    }

    Local<Value> emitValue = maybeEmitValue.ToLocalChecked();
    if (!emitValue->IsFunction()) {
      qWarning() << "emit is not function";
      return;
    }

    Local<Function> emitFn = Local<Function>::Cast(emitValue);

    Local<Value> argv[MAX_ARGS_COUNT_FOR_SIGNAL];
    argv[0] = String::NewFromUtf8(isolate, signal.toUtf8().constData(), v8::NewStringType::kNormal)
                  .ToLocalChecked();
    for (int i = 0; i < qMin(args.size(), MAX_ARGS_COUNT_FOR_SIGNAL - 1); i++) {
      argv[i + 1] = JSObjectHelper::toV8Value(args[i], isolate);
    }

    TryCatch trycatch(isolate);
    // When an exception occurs, Function::Call returns empty value.
    MaybeLocal<Value> maybeResult =
        emitFn->Call(isolate->GetCurrentContext(), *jsObj,
                     qMin(args.size() + 1, MAX_ARGS_COUNT_FOR_SIGNAL), argv);
    if (trycatch.HasCaught()) {
      MaybeLocal<Value> maybeStackTrace = trycatch.StackTrace(isolate->GetCurrentContext());
      Local<Value> exception = trycatch.Exception();
      String::Utf8Value exceptionStr(exception);
      std::stringstream ss;
      ss << "error: " << *exceptionStr;
      if (!maybeStackTrace.IsEmpty()) {
        String::Utf8Value stackTraceStr(maybeStackTrace.ToLocalChecked());
        ss << " stack trace: " << *stackTraceStr;
      }
      qWarning() << ss.str().c_str();
      return;
    } else if (maybeResult.IsEmpty()) {
      qWarning() << "maybeResult is empty (but exception is not thrown...)";
      return;
    }
  } else {
    qWarning() << "associated JS object not found";
  }
}
