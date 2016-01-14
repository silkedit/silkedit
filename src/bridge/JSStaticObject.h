#pragma once

#include <node.h>
#include <v8.h>
#include <sstream>
#include <QDebug>
#include <QMetaProperty>
#include <QDialogButtonBox>

#include "JSObjectHelper.h"
#include "ObjectStore.h"
#include "QObjectHelper.h"
#include "JSHandler.h"
#include "ObjectTemplateStore.h"
#include "core/QVariantArgument.h"
#include "core/macros.h"
#include "core/Util.h"

using core::Util;

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
      NODE_SET_PROTOTYPE_METHOD(tpl, method.name().constData(), JSObjectHelper::invokeMethod);
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
    exports->Set(v8::String::NewFromUtf8(isolate, Util::stipNamespace(metaObj.className())), ctor);
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
        variants.append(JSObjectHelper::toVariant(isolate, args[i]));
      }

      for (int i = 0; i < metaObj.constructorCount(); i++) {
        const QMetaMethod& ctor = metaObj.constructor(i);
        if (ctor.access() != QMetaMethod::Public) {
          continue;
        }

        const auto& parameterTypes = ctor.parameterTypes();
        if (JSObjectHelper::matchTypes(parameterTypes, variants)) {
          // overwrite QVariant type with parameter type to match the method signature.
          // e.g. convert QLabel* to QWidget*
          for (int j = 0; j < ctor.parameterCount(); j++) {
            variants[j] = QVariant(QMetaType::type(parameterTypes[j]), variants[j].data());
          }

          QObject* newObj = QObjectHelper::singleton().newInstanceFromJS(metaObj, variants);
          if (!newObj) {
            std::stringstream ss;
            ss << "invoking" << ctor.methodSignature().constData() << "failed";
            isolate->ThrowException(v8::Exception::TypeError(
                v8::String::NewFromUtf8(isolate, ss.str().c_str(), v8::NewStringType::kNormal)
                    .ToLocalChecked()));
            return;
          }

          ObjectStore::singleton().wrapAndInsert(newObj, args.This(), isolate);
          args.GetReturnValue().Set(args.This());
          return;
        }
      }

      std::stringstream ss;
      ss << "no constructor matched. args.Length:" << args.Length();
      // no constructor matched
      isolate->ThrowException(v8::Exception::TypeError(
          v8::String::NewFromUtf8(isolate, ss.str().c_str(), v8::NewStringType::kNormal)
              .ToLocalChecked()));
    } else {
      // Invoked as plain function `MyObject(...)`, turn into construct call.
      if (args.Length() >= MAX_ARGS_COUNT) {
        qWarning() << "max # of args is " << MAX_ARGS_COUNT
                   << ". Exceeded arguments will be dropped";
      }

      v8::Local<v8::Value> argv[MAX_ARGS_COUNT];
      for (int i = 0; i < qMin(args.Length(), MAX_ARGS_COUNT); i++) {
        argv[i] = args[i];
      }
      v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(isolate, constructor);
      args.GetReturnValue().Set(cons->NewInstance(qMin(args.Length(), MAX_ARGS_COUNT), argv));
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
