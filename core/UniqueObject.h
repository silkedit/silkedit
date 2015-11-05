#pragma once

#include <limits>
#include <QHash>
#include <QMutex>

#include "msgpack/rpc/protocol.h"

namespace core {

// Trait to assign unique id for each instance of T using CRTP
// http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
template <typename T>
struct UniqueObject {
  UniqueObject() : m_id(-1) {}

  static T* find(int id) {
    if (id < 0) {
      qWarning("id must be positive");
      return nullptr;
    }

    QMutexLocker locker(&s_mutex);
    if (s_objects.contains(id)) {
      return s_objects.value(id);
    }

    return nullptr;
  }

  static void callNotifyFunc(const QString& method, const msgpack::object& obj) {
    std::tuple<int> params;
    obj.convert(&params);
    int id = std::get<0>(params);

    if (T* view = UniqueObject::find(id)) {
      T::notify(view, method, obj);
    } else {
      qWarning("id: %d not found", id);
    }
  }

  static void callRequestFunc(msgpack::rpc::msgid_t msgId,
                              const QString& method,
                              const msgpack::object& obj) {
    std::tuple<int> params;
    obj.convert(&params);
    int id = std::get<0>(params);

    if (T* view = UniqueObject::find(id)) {
      T::request(view, method, msgId, obj);
    } else {
      qWarning("id: %d not found", id);
    }
  }

  int id() {
    if (m_id >= 0) {
      return m_id;
    }

    m_id = nextId();
    QMutexLocker locker(&s_mutex);
    s_objects.insert(m_id, static_cast<T*>(this));
    return m_id;
  }

 protected:
  virtual ~UniqueObject()  // objects should never be removed through pointers of this type
  {
    //    qDebug("~UniqueObject");
    if (m_id >= 0) {
      QMutexLocker locker(&s_mutex);
      s_objects.remove(m_id);
    }
  }

 private:
  static uint s_count;
  static QHash<int, T*> s_objects;
  static QMutex s_mutex;

  static int nextId() {
    if (s_objects.keys().size() == std::numeric_limits<int>::max()) {
      throw std::runtime_error("s_objects has no capacity to store a new id");
    }

    while (s_objects.count(s_count) != 0) {
      s_count++;
    }

    return s_count;
  }

  int m_id;
};

template <typename T>
uint UniqueObject<T>::s_count(0);
template <typename T>
QHash<int, T*> UniqueObject<T>::s_objects;
template <typename T>
QMutex UniqueObject<T>::s_mutex;

}  // namespace core
