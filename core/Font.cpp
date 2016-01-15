#include <sstream>
#include <QMetaMethod>
#include <QDebug>

#include "Font.h"
#include "QObjectUtil.h"
#include "Util.h"
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

namespace core {

v8::Persistent<v8::Function> Font::constructor;

void Font::Init(v8::Local<v8::Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Font"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "Font"), tpl->GetFunction());
}

void Font::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.IsConstructCall()) {
    const QMetaObject& metaObj = Font::staticMetaObject;

    // convert args to QVariantList
    QVariantList variants;
    for (int i = 0; i < args.Length(); i++) {
      variants.append(V8Util::toVariant(isolate, args[i]));
    }

    for (int i = 0; i < metaObj.constructorCount(); i++) {
      const QMetaMethod& ctor = metaObj.constructor(i);
      if (ctor.access() != QMetaMethod::Public) {
        continue;
      }

      const auto& parameterTypes = ctor.parameterTypes();
      if (Util::matchTypes(parameterTypes, variants)) {
        // overwrite QVariant type with parameter type to match the method signature.
        // e.g. convert QLabel* to QWidget*
        for (int j = 0; j < ctor.parameterCount(); j++) {
          variants[j] = QVariant(QMetaType::type(parameterTypes[j]), variants[j].data());
        }

        Font* newObj = static_cast<Font*>(
            QObjectUtil::newInstanceOfGadgetFromJS(metaObj, variants));
        if (!newObj) {
          std::stringstream ss;
          ss << "invoking" << ctor.methodSignature().constData() << "failed";
          V8Util::throwError(isolate, ss.str());
          return;
        }

        newObj->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
        return;
      }
    }

    std::stringstream ss;
    ss << "no constructor matched. args.Length:" << args.Length();
    // no constructor matched
    V8Util::throwError(isolate, ss.str());
  } else {
    // Invoked as plain function `MyObject(...)`, turn into construct call.
    if (args.Length() >= Q_METAMETHOD_INVOKE_MAX_ARGS) {
      qWarning() << "max # of args is " << Q_METAMETHOD_INVOKE_MAX_ARGS
                 << ". Exceeded arguments will be dropped";
    }

    v8::Local<v8::Value> argv[Q_METAMETHOD_INVOKE_MAX_ARGS];
    for (int i = 0; i < qMin(args.Length(), Q_METAMETHOD_INVOKE_MAX_ARGS); i++) {
      argv[i] = args[i];
    }
    v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(qMin(args.Length(), Q_METAMETHOD_INVOKE_MAX_ARGS), argv));
  }
}

}  // namespace core

