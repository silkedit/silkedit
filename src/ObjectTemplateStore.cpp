#include <QMetaProperty>
#include <QDebug>
#include <sstream>

#include "ObjectTemplateStore.h"
#include "JSObjectHelper.h"
#include "QObjectHelper.h"
#include "ObjectStore.h"
#include "core/v8adapter.h"

using v8::UniquePersistent;
using v8::ObjectTemplate;
using v8::EscapableHandleScope;
using v8::Local;
using v8::String;
using v8::PropertyCallbackInfo;
using v8::Value;
using v8::Isolate;
using v8::Exception;

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
  Local<ObjectTemplate> objTempl = ObjectTemplate::New(isolate);
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

v8::Local<v8::ObjectTemplate> ObjectTemplateStore::getObjectTemplate(const QMetaObject* metaObj,
                                                                     v8::Isolate* isolate) {
  EscapableHandleScope handle_scope(isolate);
  if (m_classObjectTemplateHash.count(metaObj) != 0) {
    return handle_scope.Escape(m_classObjectTemplateHash.at(metaObj).Get(isolate));
  } else {
    Local<ObjectTemplate> objTempl = createObjectTemplate(metaObj, isolate);

    // cache object template
    UniquePersistent<ObjectTemplate> persistentTempl(isolate, objTempl);
    auto pair = std::make_pair(metaObj, std::move(persistentTempl));
    m_classObjectTemplateHash.insert(std::move(pair));
    return handle_scope.Escape(objTempl);
  }
}

void ObjectTemplateStore::getterCallback(Local<String> property,
                                         const PropertyCallbackInfo<Value>& info) {
  Isolate* isolate = info.GetIsolate();
  QObject* obj = ObjectStore::unwrap(info.Holder());
  if (!obj) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "QObject is null")));
    return;
  }

  String::Utf8Value propertyName(property);
  int propertyIndex = obj->metaObject()->indexOfProperty(*propertyName);
  if (propertyIndex == -1) {
    std::stringstream ss;
    ss << "property:" << *propertyName << "not found";
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, ss.str().data())));
    return;
  }

  QMetaProperty prop = obj->metaObject()->property(propertyIndex);
  if (!prop.isReadable()) {
    std::stringstream ss;
    ss << "property:" << *propertyName << "is not readable";
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, ss.str().c_str())));
    return;
  }

  QVariant value = prop.read(obj);
  info.GetReturnValue().Set(JSObjectHelper::toV8Value(isolate, value));
}

void ObjectTemplateStore::setterCallback(v8::Local<v8::String> property,
                                         v8::Local<v8::Value> value,
                                         const v8::PropertyCallbackInfo<void>& info) {
  Isolate* isolate = info.GetIsolate();
  QObject* obj = ObjectStore::unwrap(info.Holder());
  if (!obj) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "QObject is null")));
    return;
  }

  String::Utf8Value propertyName(property);
  int propertyIndex = obj->metaObject()->indexOfProperty(*propertyName);
  if (propertyIndex == -1) {
    std::stringstream ss;
    ss << "property:" << *propertyName << "not found";
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, ss.str().c_str())));
    return;
  }

  QMetaProperty prop = obj->metaObject()->property(propertyIndex);
  if (!prop.isWritable()) {
    std::stringstream ss;
    ss << "property:" << *propertyName << "is not writable";
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, ss.str().c_str())));
    return;
  }

  prop.write(obj, JSObjectHelper::toVariant(isolate, value));
}
