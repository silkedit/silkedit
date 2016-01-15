#pragma once

#include <node.h>
#include <v8.h>
#include <sstream>
#include <QDebug>
#include <QMetaProperty>
#include <QDialogButtonBox>

#include "JSObjectHelper.h"
#include "core/JSHandler.h"
#include "core/QObjectHelper.h"
#include "core/ObjectTemplateStore.h"
#include "core/ObjectStore.h"
#include "core/QVariantArgument.h"
#include "core/macros.h"
#include "core/Util.h"
#include "core/V8Util.h"

using core::Util;
using core::QObjectHelper;
using core::ObjectTemplateStore;
using core::JSHandler;
using core::V8Util;

namespace bridge {

template <class QObjectSubClass>
class JSStaticObject {
 public:
  static v8::Local<v8::Function> Init(v8::Local<v8::Object> exports) {
    v8::Isolate* isolate = exports->GetIsolate();
    const QMetaObject& metaObj = QObjectSubClass::staticMetaObject;

    // Prepare constructor template
    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(isolate, New);
    tpl->SetClassName(v8::String::NewFromUtf8(isolate, metaObj.className()));
    ObjectTemplateStore::initInstanceTemplate(tpl->InstanceTemplate(), &metaObj, isolate);

    // register method and slots to prototype object
    Util::processWithPublicMethods(&metaObj, [&](const QMetaMethod& method) {
      NODE_SET_PROTOTYPE_METHOD(tpl, method.name().constData(), V8Util::invokeMethod);
    });

    v8::MaybeLocal<v8::Function> maybeFunc = tpl->GetFunction(isolate->GetCurrentContext());
    if (maybeFunc.IsEmpty()) {
      // This happens when /usr/local/lib/libnode.dylib is debug version
      qCritical() << "Failed to GetFunction of FunctionTemplate";
      return;
    }
    v8::Local<v8::Function> ctor = maybeFunc.ToLocalChecked();

    addEnumsToFunction(metaObj, ctor, isolate);
    JSHandler::inheritsQtEventEmitter(isolate, ctor);
    constructor.Reset(isolate, ctor);
    exports->Set(v8::String::NewFromUtf8(isolate, Util::stripNamespace(metaObj.className())), ctor);
    return ctor;
  }

 private:
  static v8::Persistent<v8::Function> constructor;

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();

    if (args.IsConstructCall()) {
      const QMetaObject& metaObj = QObjectSubClass::staticMetaObject;

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

          QObject* newObj = QObjectHelper::singleton().newInstanceFromJS(metaObj, variants);
          if (!newObj) {
            std::stringstream ss;
            ss << "invoking" << ctor.methodSignature().constData() << "failed";
            V8Util::throwError(isolate, ss.str());
            return;
          }

          core::ObjectStore::singleton().wrapAndInsert(newObj, args.This(), isolate);
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
      args.GetReturnValue().Set(cons->NewInstance(qMin(args.Length(), Q_METAMETHOD_INVOKE_MAX_ARGS), argv));
    }
  }

  static void addEnumsToFunction(const QMetaObject& metaObj,
                                 v8::Local<v8::Function> ctor,
                                 v8::Isolate* isolate) {
    for (int i = 0; i < metaObj.enumeratorCount(); i++) {
      const QMetaEnum& metaEnum = metaObj.enumerator(i);
      v8::Local<v8::Object> enumObj = v8::Object::New(isolate);
      for (int j = 0; j < metaEnum.keyCount(); j++) {
        enumObj->Set(v8::String::NewFromUtf8(isolate, metaEnum.key(j)),
                     v8::Integer::New(isolate, metaEnum.value(j)));
      }
      ctor->Set(v8::String::NewFromUtf8(isolate, metaEnum.name()), enumObj);
    }
  }

  JSStaticObject() = delete;
  ~JSStaticObject() = delete;
};

template <typename QObjectSubClass>
v8::Persistent<v8::Function> JSStaticObject<QObjectSubClass>::constructor;

}  // namespace bridge
