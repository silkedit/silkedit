#pragma once

#include <boost/optional.hpp>
#include <v8.h>
#include <unordered_map>
#include <QMetaObject>
#include <QHash>

#include "core/macros.h"
#include "core/Singleton.h"

class ObjectTemplateStore : public core::Singleton<ObjectTemplateStore> {
  DISABLE_COPY(ObjectTemplateStore)

 public:
  static v8::Local<v8::ObjectTemplate> createObjectTemplate(const QMetaObject* metaObj,
                                                            v8::Isolate* isolate);
  static void initInstanceTemplate(v8::Local<v8::ObjectTemplate> objTempl,
                                   const QMetaObject* metaObj,
                                   v8::Isolate* isolate);

  ~ObjectTemplateStore() = default;
  DEFAULT_MOVE(ObjectTemplateStore)

  v8::Local<v8::ObjectTemplate> getObjectTemplate(const QMetaObject* metaObj, v8::Isolate* isolate);

 private:
  static QHash<const QMetaObject*, QHash<QString, int>> s_classPropertiesHash;
  static void cacheProperties(const QMetaObject* metaObj);
  static void getterCallback(v8::Local<v8::String> property,
                             const v8::PropertyCallbackInfo<v8::Value>& info);
  static void setterCallback(v8::Local<v8::String> property,
                                         v8::Local<v8::Value> value,
                                         const v8::PropertyCallbackInfo<void>& info);

  std::unordered_map<const QMetaObject*, v8::UniquePersistent<v8::ObjectTemplate>>
      m_classObjectTemplateHash;

  friend class core::Singleton<ObjectTemplateStore>;
  ObjectTemplateStore() = default;
};
