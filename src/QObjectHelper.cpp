#include <QMetaMethod>
#include <QDebug>

#include "QObjectHelper.h"
#include "ObjectStore.h"
#include "core/QVariantArgument.h"

using core::QVariantArgument;

QObject* QObjectHelper::newInstanceFromJS(const QMetaObject& metaObj, QVariantList args) {
  qDebug() << "newInstanceFromJS of" << metaObj.className();
  if (args.size() > Q_METAMETHOD_INVOKE_MAX_ARGS) {
    qWarning() << "Can't invoke constructor with more than" << Q_METAMETHOD_INVOKE_MAX_ARGS
               << "arguments. args:" << args.size();
    return nullptr;
  }
  QVariantArgument varArgs[Q_METAMETHOD_INVOKE_MAX_ARGS];
  for (int i = 0; i < args.size(); i++) {
    varArgs[i].value = args[i];
  }
  QObject* obj = metaObj.newInstance(varArgs[0], varArgs[1], varArgs[2], varArgs[3], varArgs[4],
                                     varArgs[5], varArgs[6], varArgs[7], varArgs[8], varArgs[9]);
  if (obj) {
    obj->setProperty(OBJECT_STATE, QVariant::fromValue(ObjectStore::NewFromJS));
  }
  return obj;
}

QVariant QObjectHelper::read(QObject* obj, const QMetaProperty& prop) {
  return prop.read(obj);
}

void QObjectHelper::write(QObject* obj, const QMetaProperty& prop, const QVariant& value) {
  prop.write(obj, value);
}
