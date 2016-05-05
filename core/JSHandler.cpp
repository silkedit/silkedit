#include <sstream>
#include <QLoggingCategory>

#include "JSHandler.h"
#include "ObjectStore.h"
#include "CommandArgument.h"
#include "V8Util.h"
#include "atom/node_includes.h"

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
using v8::Locker;

namespace core {

v8::Persistent<v8::Object> JSHandler::s_jsHandler;
bool JSHandler::s_isInitialized = false;

namespace {
constexpr int MAX_ARGS_COUNT_FOR_SIGNAL = MAX_ARGS_COUNT + 1;
}

void JSHandler::init(v8::Local<v8::Object> jsHandler) {
  s_jsHandler.Reset(Isolate::GetCurrent(), jsHandler);
  s_isInitialized = true;
}

QVariant JSHandler::callFunc(Isolate* isolate, const QString& funcName, QVariantList args) {
  if (!s_isInitialized) {
    qWarning() << "JSHandler is not yet initialized";
    return QVariant();
  }

  MaybeLocal<Value> maybeFnValue =
      s_jsHandler.Get(isolate)->Get(isolate->GetCurrentContext(), V8Util::toV8String(isolate, funcName));
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

  if (args.size() >= MAX_ARGS_COUNT) {
    qWarning() << "max # of args is " << MAX_ARGS_COUNT << ". Exceeded arguments will be dropped";
  }

  Local<Value> argv[MAX_ARGS_COUNT];
  int argc = qMin(args.size(), MAX_ARGS_COUNT);
  for (int i = 0; i < argc; i++) {
    argv[i] = V8Util::toV8Value(isolate, args[i]);
  }

  return V8Util::callJSFunc(isolate, fn, s_jsHandler.Get(isolate), argc, argv);
}

void JSHandler::inheritsQtEventEmitter(Isolate* isolate, Local<v8::Value> proto) {
  Q_ASSERT(s_isInitialized);

  Locker locker(isolate);
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

void JSHandler::emitSignal(Isolate* isolate, QObject* obj, const QString& signal, QVariantList args) {
  qDebug() << "emitSignal. " << signal << "args:" << args;

  if (!s_isInitialized) {
    qWarning() << "JSHandler is not yet initialized";
    return;
  }

  if (args.size() >= MAX_ARGS_COUNT) {
    qWarning() << "max # of args is " << MAX_ARGS_COUNT << ". Exceeded arguments will be dropped";
  }

  if (const auto& jsObj = ObjectStore::singleton().find(obj, isolate)) {
    MaybeLocal<Value> maybeEmitValue = (*jsObj)->Get(
        isolate->GetCurrentContext(),
        String::NewFromUtf8(isolate, "_emit", v8::NewStringType::kNormal).ToLocalChecked());
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
      argv[i + 1] = V8Util::toV8Value(isolate, args[i]);
    }

    TryCatch trycatch(isolate);
    // When an exception occurs, Function::Call returns empty value.
    MaybeLocal<Value> maybeResult =
        emitFn->Call(isolate->GetCurrentContext(), *jsObj,
                     qMin(args.size() + 1, MAX_ARGS_COUNT_FOR_SIGNAL), argv);
    if (trycatch.HasCaught()) {
      QLoggingCategory category("silkedit");
      const auto& msg = V8Util::getErrorMessage(isolate, trycatch);
      qCCritical(category).noquote() << msg;

      return;
    } else if (maybeResult.IsEmpty()) {
      QLoggingCategory category("silkedit");
      qCCritical(category) << "maybeResult is empty (but exception is not thrown...)";
      return;
    }
  } else {
    qWarning() << "associated JS object not found";
  }
}

}  // namespace core
