#include <sstream>
#include <QMetaMethod>

#include "JSObjectHelper.h"
#include "Helper.h"
#include "core/ObjectStore.h"
#include "core/V8Util.h"

using core::ObjectStore;
using core::V8Util;

using v8::String;
using v8::Value;
using v8::Isolate;
using v8::FunctionCallbackInfo;

namespace {

QByteArray parameterTypeSignature(const QByteArray& methodSignature) {
  return methodSignature.mid(std::max(0, methodSignature.indexOf('(')));
}

}

void JSObjectHelper::connect(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);

  if (args.Length() <= 0 || !args[0]->IsString()) {
    V8Util::throwError(isolate, "invalid argument");
    return;
  }

  QObject* obj = ObjectStore::unwrap(args.Holder());
  if (!obj) {
    V8Util::throwError(isolate, "can't convert to QObject");
    return;
  }

  v8::String::Utf8Value eventNameValue(
      args[0]->ToString(isolate->GetCurrentContext()).ToLocalChecked());
  const char* eventName = *eventNameValue;

  const QMetaObject* metaObj = obj->metaObject();
  int index = -1;
  for (int i = 0; i < metaObj->methodCount(); i++) {
    if (metaObj->method(i).methodType() == QMetaMethod::Signal &&
        metaObj->method(i).name() == eventName) {
      index = i;
      break;
    }
  }

  if (index < 0) {
    std::stringstream ss;
    ss << eventName << " is not defined";
    V8Util::throwError(isolate, ss.str());
    return;
  }

  const QMetaMethod& method = metaObj->method(index);
  if (method.name() == "destoryed") {
    V8Util::throwError(isolate, "add listener to destroyed event is not allowed");
    return;
  }

  //  qDebug() << method.methodSignature();
  QByteArray emitSignalSignature =
      parameterTypeSignature(method.methodSignature()).prepend("emitSignal");
  //  qDebug() << emitSignalSignature;
  const QMetaMethod& emitSignal =
      Helper::staticMetaObject.method(Helper::staticMetaObject.indexOfMethod(emitSignalSignature));
  if (emitSignal.isValid()) {
    QObject::connect(obj, method, &Helper::singleton(), emitSignal, Qt::UniqueConnection);
  } else {
    std::stringstream ss;
    ss << "parameter signature" << emitSignalSignature.constData() << "not supported";
    V8Util::throwError(isolate, ss.str());
    return;
  }
}
