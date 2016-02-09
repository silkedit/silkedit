#pragma once

#include <v8.h>
#include <functional>
#include <string>
#include <QString>
#include <QVariant>
#include <QCache>
#include <QMultiHash>
#include <QKeyEvent>

#include "CommandArgument.h"
#include "Util.h"
#include "macros.h"

namespace core {

constexpr int MAX_ARGS_COUNT = 10;

// static class
class V8Util {
  V8Util() = delete;
  ~V8Util() = delete;

 public:
  static v8::Local<v8::String> hiddenQObjectKey(v8::Isolate* isolate);
  static v8::Local<v8::String> constructorKey(v8::Isolate* isolate);

  static QString toQString(v8::Local<v8::String> str) {
    return QString::fromUtf16(*v8::String::Value(str));
  }

  static std::string toStdString(v8::Local<v8::String> str) {
    v8::String::Utf8Value value(str);
    return *value;
  }

  static v8::Local<v8::String> toV8String(v8::Isolate* isolate, const QString& str) {
    return v8::String::NewFromTwoByte(isolate, str.utf16());
  }

  static v8::Local<v8::String> toV8String(v8::Isolate* isolate, const std::string& str) {
    return v8::String::NewFromUtf8(isolate, str.c_str());
  }

  static QVariant toVariant(v8::Isolate* isolate, v8::Local<v8::Value> value);

  static v8::Local<v8::Value> toV8Value(v8::Isolate* isolate, const QVariant& var);

  static v8::Local<v8::Object> toV8Object(v8::Isolate* isolate, const CommandArgument args);

  static v8::Local<v8::Value> toV8ObjectFrom(v8::Isolate* isolate, QObject* sourceObj);

  static QVariantMap toVariantMap(v8::Isolate* isolate, v8::Local<v8::Object> obj);

  static void throwError(v8::Isolate* isolate, const std::string& msg);
  static void throwError(v8::Isolate* isolate, const char* msg);

  static void invokeQObjectMethod(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void emitQObjectSignal(const v8::FunctionCallbackInfo<v8::Value>& args);

  static QVariant callJSFunc(v8::Isolate* isolate,
                             v8::Local<v8::Function> fn,
                             v8::Local<v8::Value> recv,
                             int argc,
                             v8::Local<v8::Value> argv[]);

  static bool checkArguments(const v8::FunctionCallbackInfo<v8::Value> args,
                             int numArgs,
                             std::function<bool()> validateFn);

  static QString getErrorMessage(v8::Isolate *isolate, const v8::TryCatch &trycatch);
private:
  friend class V8UtilTest;

  static v8::Persistent<v8::String> s_hiddenQObjectKey;
  static v8::Persistent<v8::String> s_constructorKey;

  static void cacheMethods(const QMetaObject* metaObj);
  static v8::MaybeLocal<v8::Object> newInstance(v8::Isolate* isolate,
                                                v8::Local<v8::Function> constructor,
                                                void* sourceObj);
};

}  // namespace core
