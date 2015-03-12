#pragma once

#include <msgpack/rpc/protocol.h>
#include <QHash>
#include <QMutex>

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

    QMutexLocker locker(&mutex);
    if (objects.contains(id)) {
      return objects.value(id);
    }

    return nullptr;
  }

  static void callNotifyFunc(const std::string& method, const msgpack::object& obj) {
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
                              const std::string& method,
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
    QMutexLocker locker(&mutex);
    objects.insert(s_count, static_cast<T*>(this));
    m_id = s_count;
    return s_count++;
  }

 protected:
  virtual ~UniqueObject()  // objects should never be removed through pointers of this type
  {
    qDebug("~UniqueObject");
    if (m_id >= 0) {
      QMutexLocker locker(&mutex);
      objects.remove(m_id);
    }
  }

 private:
  static int s_count;
  static QHash<int, T*> objects;
  static QMutex mutex;

  int m_id;
};

template <typename T>
int UniqueObject<T>::s_count(0);
template <typename T>
QHash<int, T*> UniqueObject<T>::objects;
template <typename T>
QMutex UniqueObject<T>::mutex;
