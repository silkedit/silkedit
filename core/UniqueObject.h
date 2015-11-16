#pragma once

#include <limits>
#include <QHash>
#include <QMutex>
#include <QMetaObject>
#include <QMetaMethod>
#include <QMetaType>
#include <QObject>
#include <QVariantList>

#include "msgpack/rpc/protocol.h"
#include "ArgumentArray.h"

namespace core {

class UniqueObject {
 public:
  UniqueObject() : m_id(-1) {}

  static QObject* find(int id) {
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

  int id() {
    if (m_id >= 0) {
      return m_id;
    }

    m_id = nextId();
    QMutexLocker locker(&s_mutex);
    s_objects.insert(m_id, dynamic_cast<QObject*>(this));
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
  static QHash<int, QObject*> s_objects;
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

  // todo: use UUID
  int m_id;
};

}  // namespace core
