#include "UniqueObject.h"

QHash<QUuid, QObject*> core::UniqueObject::s_objects;
QMutex core::UniqueObject::s_mutex;

void core::UniqueObject::registerObj(const QUuid& id, QObject* obj) {
  QMutexLocker locker(&s_mutex);
  s_objects.insert(id, obj);
}

QObject* core::UniqueObject::find(const QUuid& id) {
  if (id.isNull()) {
    qWarning("uuid is null");
    return nullptr;
  }

  QMutexLocker locker(&s_mutex);
  if (s_objects.contains(id)) {
    return s_objects.value(id);
  }

  return nullptr;
}

QUuid core::UniqueObject::id() {
  if (!m_id.isNull()) {
    return m_id;
  }

  m_id = QUuid::createUuid();
  QMutexLocker locker(&s_mutex);
  s_objects.insert(m_id, dynamic_cast<QObject*>(this));
  return m_id;
}

core::UniqueObject::~UniqueObject() {
  //    qDebug("~UniqueObject");
  if (!m_id.isNull()) {
    QMutexLocker locker(&s_mutex);
    s_objects.remove(m_id);
  }
}
