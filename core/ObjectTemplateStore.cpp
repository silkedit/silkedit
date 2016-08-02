#include <QMetaProperty>
#include <QDebug>
#include <sstream>

#include "ObjectTemplateStore.h"
#include "V8Util.h"
#include "ObjectStore.h"
#include "Util.h"

using core::Util;

using v8::UniquePersistent;
using v8::ObjectTemplate;
using v8::EscapableHandleScope;
using v8::Local;
using v8::String;
using v8::PropertyCallbackInfo;
using v8::Value;
using v8::Isolate;
using v8::Exception;

namespace core {

QHash<const QMetaObject*, QHash<QString, int>> ObjectTemplateStore::s_classPropertiesHash;

void ObjectTemplateStore::cacheProperties(const QMetaObject* metaObj) {
  QHash<QString, int> propertyNameIndexHash;
  for (int i = 0; i < metaObj->propertyCount(); i++) {
    const QString& name = QString::fromLatin1(metaObj->property(i).name());
    propertyNameIndexHash.insert(name, i);
  }
  s_classPropertiesHash.insert(metaObj, propertyNameIndexHash);
}

v8::Local<v8::ObjectTemplate> ObjectTemplateStore::createObjectTemplate(const QMetaObject* metaObj,
                                                                        v8::Isolate* isolate) {
  v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(isolate);
  tpl->SetClassName(v8::String::NewFromUtf8(isolate, Util::stripNamespace(metaObj->className())));
  Local<ObjectTemplate> objTempl = tpl->InstanceTemplate();
  initInstanceTemplate(objTempl, metaObj, isolate);
  return objTempl;
}

void ObjectTemplateStore::initInstanceTemplate(Local<ObjectTemplate> objTempl,
                                               const QMetaObject* metaObj,
                                               v8::Isolate* isolate) {
  objTempl->SetInternalFieldCount(1);
  // set accessors
  for (int i = 0; i < metaObj->propertyCount(); i++) {
    objTempl->SetAccessor(String::NewFromUtf8(isolate, metaObj->property(i).name()), getterCallback,
                          setterCallback);
  }
}

void ObjectTemplateStore::addObjectTemplate(Local<ObjectTemplate> objTempl,
                                            const QMetaObject* metaObj,
                                            v8::Isolate* isolate) {
  UniquePersistent<ObjectTemplate> persistentTempl(isolate, objTempl);
  auto pair = std::make_pair(metaObj, std::move(persistentTempl));
  m_classObjectTemplateHash.insert(std::move(pair));
}

v8::Local<v8::ObjectTemplate> ObjectTemplateStore::getObjectTemplate(const QMetaObject* metaObj,
                                                                     v8::Isolate* isolate) {
  EscapableHandleScope handle_scope(isolate);
  if (m_classObjectTemplateHash.count(metaObj) != 0) {
    return handle_scope.Escape(m_classObjectTemplateHash.at(metaObj).Get(isolate));
  } else {
    Local<ObjectTemplate> objTempl = createObjectTemplate(metaObj, isolate);

    // cache object template
    addObjectTemplate(objTempl, metaObj, isolate);
    return handle_scope.Escape(objTempl);
  }
}

void ObjectTemplateStore::getterCallback(Local<String> property,
                                         const PropertyCallbackInfo<Value>& info) {
  Isolate* isolate = info.GetIsolate();
  QObject* obj = ObjectStore::unwrap(info.Holder());
  if (!obj) {
    V8Util::throwError(isolate, "no associated QObject");
    return;
  }

  String::Utf8Value propertyName(property);
  int propertyIndex = obj->metaObject()->indexOfProperty(*propertyName);

  // If obj is already destroyed, propertyIndex becomes -1.
  if (propertyIndex == -1) {
    info.GetReturnValue().Set(v8::Undefined(isolate));
    return;
  }

  QMetaProperty prop = obj->metaObject()->property(propertyIndex);
  if (!prop.isReadable()) {
    std::stringstream ss;
    ss << "property: " << *propertyName << " is not readable";
    V8Util::throwError(isolate, ss.str());
    return;
  }

  QVariant value = prop.read(obj);
  info.GetReturnValue().Set(V8Util::toV8Value(isolate, value));
}

void ObjectTemplateStore::setterCallback(v8::Local<v8::String> property,
                                         v8::Local<v8::Value> value,
                                         const v8::PropertyCallbackInfo<void>& info) {
  Isolate* isolate = info.GetIsolate();
  QObject* obj = ObjectStore::unwrap(info.Holder());
  if (!obj) {
    V8Util::throwError(isolate, "no associated QObject");
    return;
  }

  String::Utf8Value propertyName(property);
  int propertyIndex = obj->metaObject()->indexOfProperty(*propertyName);
  if (propertyIndex == -1) {
    std::stringstream ss;
    ss << "property: " << *propertyName << " not found";
    V8Util::throwError(isolate, ss.str());
    return;
  }

  QMetaProperty prop = obj->metaObject()->property(propertyIndex);
  if (!prop.isWritable()) {
    std::stringstream ss;
    ss << "property: " << *propertyName << " is not writable";
    V8Util::throwError(isolate, ss.str());
    return;
  }

  prop.write(obj, V8Util::toVariant(isolate, value));
}

}  // namespace core
