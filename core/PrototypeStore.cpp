#include <node.h>
#include <QMetaMethod>

#include "PrototypeStore.h"
#include "JSHandler.h"
#include "Util.h"

using v8::UniquePersistent;
using v8::Object;
using v8::EscapableHandleScope;
using v8::Local;

void core::PrototypeStore::insert(const QMetaObject* metaObj,
                                  v8::Local<v8::Object> proto,
                                  v8::Isolate* isolate) {
  UniquePersistent<Object> persistentPrototype(isolate, proto);
  auto pair = std::make_pair(metaObj, std::move(persistentPrototype));
  m_prototypeHash.insert(std::move(pair));
}

boost::optional<v8::Local<v8::Object>> core::PrototypeStore::find(const QMetaObject* metaObj,
                                                                  v8::Isolate* isolate) {
  if (m_prototypeHash.count(metaObj) != 0) {
    return m_prototypeHash.at(metaObj).Get(isolate);
  } else {
    return boost::none;
  }
}

v8::Local<v8::Object> core::PrototypeStore::getOrCreatePrototype(const QMetaObject* metaObj,
                                                         v8::FunctionCallback invoke,
                                                         v8::Isolate* isolate) {
  EscapableHandleScope handle_scope(isolate);
  if (const auto& maybeProto = PrototypeStore::singleton().find(metaObj, isolate)) {
    return handle_scope.Escape(*maybeProto);
  } else {
    // cache prototype object
    Local<Object> proto = Object::New(isolate);
    Util::processWithPublicMethods(metaObj, [&](const QMetaMethod& method) {
      NODE_SET_METHOD(proto, method.name().constData(), invoke);
    });
    JSHandler::inheritsQtEventEmitter(isolate, proto);
    PrototypeStore::singleton().insert(metaObj, proto);
    return handle_scope.Escape(proto);
  }
}
