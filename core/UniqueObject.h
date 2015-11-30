#pragma once

#include <limits>
#include <QHash>
#include <QMutex>
#include <QMetaObject>
#include <QMetaMethod>
#include <QMetaType>
#include <QObject>
#include <QVariantList>
#include <QUuid>

#include "msgpack/rpc/protocol.h"
#include "ArgumentArray.h"

namespace core {

class UniqueObject {
 public:
  static void registerObj(const QUuid& id, QObject* obj);

  static QObject* find(const QUuid& id);

  UniqueObject() = default;

  QUuid id();

 protected:
  virtual ~UniqueObject();

 private:
  static QHash<QUuid, QObject*> s_objects;
  static QMutex s_mutex;

  QUuid m_id;
};

}  // namespace core
