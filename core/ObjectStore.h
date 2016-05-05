#pragma once

#include <v8.h>
#include <boost/optional.hpp>
#include <unordered_map>
#include <unordered_set>
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

  static QObject* unwrap(v8::Local<v8::Object> obj);
  static void wrapAndInsert(QObject* obj, v8::Local<v8::Object> jsObj, v8::Isolate* isolate);
  static void registerDestroyedConnectedObject(QObject* obj);
  static boost::optional<v8::Local<v8::Object>> find(QObject* obj, v8::Isolate* isolate);
  static void clearAssociatedJSObjects();

  ~ObjectStore() = default;

 private:
  static std::unordered_map<QObject*, v8::UniquePersistent<v8::Object>> s_objects;

  /**
   * @brief s_destroyedConnectedObjects
   * Keep track of QObjects connected to destroyed signal in JS side to make sure that when QObject
   * is destroyed, emitting destroyed signal to JS side first then destroy JS object.
   */
  static std::unordered_set<QObject*> s_destroyedConnectedObjects;

  static void removeAssociatedJSObject(QObject *destroyedObj);

  static void WeakCallback(const v8::WeakCallbackData<v8::Object, QObject>& data);

  friend class Singleton<ObjectStore>;
  ObjectStore() = default;

};

}  // namespace core
