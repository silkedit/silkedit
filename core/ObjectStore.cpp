#include "ObjectStore.h"

void core::ObjectStore::insert(const QUuid &id, QObject *obj)
{
  QMutexLocker locker(&m_mutex);
  m_objects.insert(id, obj);
}

void core::ObjectStore::remove(const QUuid &id)
{
  if (!id.isNull()) {
    QMutexLocker locker(&m_mutex);
    m_objects.remove(id);
    emit objectRemoved(id);
  }
}

QObject *core::ObjectStore::find(const QUuid &id)
{
  if (id.isNull()) {
    qWarning("uuid is null");
    return nullptr;
  }

  QMutexLocker locker(&m_mutex);
  if (m_objects.contains(id)) {
    return m_objects.value(id);
  }

  return nullptr;
}
