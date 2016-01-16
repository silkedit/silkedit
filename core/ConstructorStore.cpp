#include <node.h>
#include <QDebug>
#include <QMetaEnum>

#include "ConstructorStore.h"
#include "JSHandler.h"
#include "ObjectTemplateStore.h"
#include "Util.h"
#include "V8Util.h"

using v8::UniquePersistent;
using v8::Function;
using v8::EscapableHandleScope;
using v8::Local;
using v8::String;
using v8::PropertyCallbackInfo;
using v8::Value;
using v8::Isolate;
using v8::Exception;
using v8::ObjectTemplate;
using v8::Isolate;

namespace core {

namespace {
void addEnumsToFunction(const QMetaObject& metaObj,
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
}

v8::Local<v8::Function> ConstructorStore::createConstructor(const QMetaObject* metaObj,
                                                            v8::Isolate* isolate,
                                                            v8::FunctionCallback newFunc) {
  v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(isolate, newFunc);
  tpl->SetClassName(v8::String::NewFromUtf8(isolate, Util::stripNamespace(metaObj->className())));
  Local<ObjectTemplate> objTempl = tpl->InstanceTemplate();
  ObjectTemplateStore::initInstanceTemplate(objTempl, metaObj, isolate);

  // register method and slots to prototype object
  Util::processWithPublicMethods(metaObj, [&](const QMetaMethod& method) {
    NODE_SET_PROTOTYPE_METHOD(tpl, method.name().constData(), V8Util::invokeMethod);
  });

  v8::MaybeLocal<v8::Function> maybeFunc = tpl->GetFunction(isolate->GetCurrentContext());
  if (maybeFunc.IsEmpty()) {
    // This happens when /usr/local/lib/libnode.dylib is debug version
    qCritical() << "Failed to GetFunction of FunctionTemplate";
    return Local<Function>();
  }
  v8::Local<v8::Function> ctor = maybeFunc.ToLocalChecked();

  addEnumsToFunction(*metaObj, ctor, isolate);
  JSHandler::inheritsQtEventEmitter(isolate, ctor);
  return ctor;
}

v8::Local<v8::Function> ConstructorStore::getConstructor(const QMetaObject* metaObj,
                                                         v8::Isolate* isolate,
                                                         v8::FunctionCallback newFunc) {
  EscapableHandleScope handle_scope(isolate);
  if (m_classConstructorHash.count(metaObj) != 0) {
    return handle_scope.Escape(
        v8::Local<v8::Function>::New(isolate, m_classConstructorHash.at(metaObj)));
  } else {
    Local<Function> ctor = createConstructor(metaObj, isolate, newFunc);

    // cache object template
    UniquePersistent<Function> persistentConstructor(isolate, ctor);
    auto pair = std::make_pair(metaObj, std::move(persistentConstructor));
    m_classConstructorHash.insert(std::move(pair));
    return handle_scope.Escape(ctor);
  }
}

}  // namespace core
