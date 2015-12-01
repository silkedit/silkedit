#pragma once

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QUuid>

#include "Singleton.h"

namespace core {

class ObjectStore : public QObject, public core::Singleton<ObjectStore> {
  Q_OBJECT

public:
  ~ObjectStore() = default;

  void insert(const QUuid& id, QObject* obj);
  void remove(const QUuid& id);

  QObject* find(const QUuid& id);

signals:
  void objectRemoved(const QUuid& id);

private:
  friend class core::Singleton<ObjectStore>;
  ObjectStore() = default;

  QHash<QUuid, QObject*> m_objects;
  QMutex m_mutex;
};

}  // namespace core
