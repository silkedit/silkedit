#pragma once

#include <v8.h>
#include <sstream>
#include <QDebug>

#include "JSObjectHelper.h"
#include "core/QObjectUtil.h"
#include "core/ObjectStore.h"
#include "core/Util.h"
#include "core/V8Util.h"
#include "core/ConstructorStore.h"

using core::Util;
using core::QObjectUtil;
using core::V8Util;
using core::ConstructorStore;
using core::ObjectStore;

namespace bridge {

template <class TypeWithMetaObject>
class JSStaticObject {
 public:
  static v8::Local<v8::Function> Init(v8::Local<v8::Object> exports) {
    v8::Isolate* isolate = exports->GetIsolate();
    const QMetaObject& metaObj = TypeWithMetaObject::staticMetaObject;
    auto ctor = ConstructorStore::singleton().createConstructor(&metaObj, isolate, New);
    exports->Set(v8::String::NewFromUtf8(isolate, Util::stripNamespace(metaObj.className())), ctor);
    return ctor;
  }

 private:
  static void New(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    const QMetaObject& metaObj = TypeWithMetaObject::staticMetaObject;

    if (args.IsConstructCall()) {
      // If this constructor is called from C++, existing C++ instance is passed as hidden value
      v8::Local<v8::Value> wrappedObj =
          args.Callee()->GetHiddenValue(V8Util::hiddenQObjectKey(isolate));
      if (!wrappedObj.IsEmpty() && wrappedObj->IsObject()) {
        QObject* sourceObj = ObjectStore::unwrap(
            wrappedObj->ToObject(isolate->GetCurrentContext()).ToLocalChecked());
        if (!sourceObj) {
          V8Util::throwError(isolate, "internal object is invalid");
          return;
        }

        ObjectStore::wrapAndInsert(sourceObj, args.This(), isolate);
        args.GetReturnValue().Set(args.This());
        return;
      }

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
          bool result = Util::convertArgs(parameterTypes, variants);
          if (!result) {
            V8Util::throwError(isolate, "invalid arguments (failed to convert)");
            return;
          }

          QObject* newObj = QObjectUtil::newInstanceFromJS(metaObj, variants);
          if (!newObj) {
            std::stringstream ss;
            ss << "invoking " << ctor.methodSignature().constData() << " failed";
            V8Util::throwError(isolate, ss.str());
            return;
          }

          ObjectStore::wrapAndInsert(newObj, args.This(), isolate);
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

      auto ctor = ConstructorStore::singleton().findConstructor(&metaObj, isolate);
      Q_ASSERT(!ctor.IsEmpty());
      args.GetReturnValue().Set(ctor->NewInstance(isolate->GetCurrentContext(),
                                                  qMin(args.Length(), Q_METAMETHOD_INVOKE_MAX_ARGS),
                                                  argv)
                                    .ToLocalChecked());
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

}  // namespace bridge
