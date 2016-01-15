#pragma once

#include <v8.h>
#include <boost/optional.hpp>
#include <unordered_map>
#include <QObject>

#include "Singleton.h"

// From Qt doc
// Note: Dynamic properties starting with "_q_" are reserved for internal purposes.
#define OBJECT_STATE "_s_object_state"

namespace core {

class ObjectStore : public QObject, public Singleton<ObjectStore> {
  Q_OBJECT

 public:
  enum ObjectState { NewFromJS, NewFromJSButHasParent };
  Q_ENUM(ObjectState)

  static QObject* unwrap(v8::Local<v8::Object> handle);

  ~ObjectStore() = default;

  void wrapAndInsert(QObject* obj,
                     v8::Local<v8::Object> jsObj,
                     v8::Isolate* isolate = v8::Isolate::GetCurrent());
  boost::optional<v8::Local<v8::Object>> find(QObject* obj,
                                              v8::Isolate* isolate = v8::Isolate::GetCurrent());

 private:
  static void WeakCallback(const v8::WeakCallbackData<v8::Object, QObject>& data);

  static std::unordered_map<QObject*, v8::UniquePersistent<v8::Object>> s_objects;

  friend class core::Singleton<ObjectStore>;
  ObjectStore() = default;

};

}  // namespace core

