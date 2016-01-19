#pragma once

#include <v8.h>
#include <unordered_map>
#include <QMetaObject>

#include "macros.h"
#include "Singleton.h"

namespace core {

class ConstructorStore : public Singleton<ConstructorStore> {
  DISABLE_COPY(ConstructorStore)

 public:
  ~ConstructorStore() = default;
  DEFAULT_MOVE(ConstructorStore)

  v8::Local<v8::Function> findOrCreateConstructor(const QMetaObject* metaObj,
                                         v8::Isolate* isolate,
                                         bool isQObject,
                                         v8::FunctionCallback newFunc = nullptr);

  v8::Local<v8::Function> createConstructor(const QMetaObject* metaObj,
                                                   v8::Isolate* isolate,
                                                   bool isQObject,
                                                   v8::FunctionCallback newFunc = nullptr);
  v8::Local<v8::Function> findConstructor(const QMetaObject* metaObj,
                                         v8::Isolate* isolate);

 private:
  static v8::Local<v8::Function> createConstructorInternal(const QMetaObject* metaObj,
                                                   v8::Isolate* isolate,
                                                   bool isQObject,
                                                   v8::FunctionCallback newFunc = nullptr);
  std::unordered_map<const QMetaObject*, v8::UniquePersistent<v8::Function>> m_classConstructorHash;

  friend class Singleton<ConstructorStore>;
  ConstructorStore() = default;
};

}  // namespace core
