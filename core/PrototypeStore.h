#pragma once

#include <boost/optional.hpp>
#include <v8.h>
#include <unordered_map>
#include <QMetaObject>

#include "macros.h"
#include "Singleton.h"

namespace core {

class PrototypeStore : public Singleton<PrototypeStore> {
  DISABLE_COPY(PrototypeStore)

 public:
  ~PrototypeStore() = default;
  DEFAULT_MOVE(PrototypeStore)

  v8::Local<v8::Object> getOrCreatePrototype(const QMetaObject* metaObj,
                                     v8::FunctionCallback invoke,
                                     v8::Isolate* isolate);

 private:
  std::unordered_map<const QMetaObject*, v8::UniquePersistent<v8::Object>> m_prototypeHash;

  friend class Singleton<PrototypeStore>;
  PrototypeStore() = default;

  void insert(const QMetaObject* metaObj,
              v8::Local<v8::Object> proto,
              v8::Isolate* isolate = v8::Isolate::GetCurrent());
  boost::optional<v8::Local<v8::Object>> find(const QMetaObject* metaObj, v8::Isolate* isolate);
};

}  // namespace core
