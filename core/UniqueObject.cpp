#include "UniqueObject.h"
#include "ObjectStore.h"

QUuid core::UniqueObject::id() {
  if (!m_id.isNull()) {
    return m_id;
  }

  m_id = QUuid::createUuid();
  ObjectStore::singleton().insert(m_id, dynamic_cast<QObject*>(this));
  return m_id;
}

core::UniqueObject::~UniqueObject() {
  //    qDebug("~UniqueObject");
  ObjectStore::singleton().remove(m_id);
}
