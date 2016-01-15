#include <QMetaMethod>
#include <QDebug>

#include "QObjectHelper.h"
#include "ObjectStore.h"
#include "QVariantArgument.h"

using core::QVariantArgument;
using core::ObjectStore;

namespace core {

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

void *QObjectHelper::newInstanceOfGadgetFromJS(const QMetaObject &metaObj, QVariantList args)
{
  qDebug() << "newInstanceOfGadgetFromJS of" << metaObj.className();
  if (args.size() > Q_METAMETHOD_INVOKE_MAX_ARGS) {
    qWarning() << "Can't invoke constructor with more than" << Q_METAMETHOD_INVOKE_MAX_ARGS
               << "arguments. args:" << args.size();
    return nullptr;
  }
  QVariantArgument varArgs[Q_METAMETHOD_INVOKE_MAX_ARGS];
  for (int i = 0; i < args.size(); i++) {
    varArgs[i].value = args[i];
  }
  return newInstanceOfGadget(metaObj, varArgs[0], varArgs[1], varArgs[2], varArgs[3], varArgs[4],
                                     varArgs[5], varArgs[6], varArgs[7], varArgs[8], varArgs[9]);
}

void* QObjectHelper::newInstanceOfGadget(const QMetaObject& metaObj,
                                         QGenericArgument val0,
                                         QGenericArgument val1,
                                         QGenericArgument val2,
                                         QGenericArgument val3,
                                         QGenericArgument val4,
                                         QGenericArgument val5,
                                         QGenericArgument val6,
                                         QGenericArgument val7,
                                         QGenericArgument val8,
                                         QGenericArgument val9) const {
  {
    QByteArray constructorName = metaObj.className();
    {
      int idx = constructorName.lastIndexOf(':');
      if (idx != -1)
        constructorName.remove(0, idx + 1);  // remove qualified part
    }
    QVarLengthArray<char, 512> sig;
    sig.append(constructorName.constData(), constructorName.length());
    sig.append('(');

    enum { MaximumParamCount = 10 };
    const char* typeNames[] = {val0.name(), val1.name(), val2.name(), val3.name(), val4.name(),
                               val5.name(), val6.name(), val7.name(), val8.name(), val9.name()};

    int paramCount;
    for (paramCount = 0; paramCount < MaximumParamCount; ++paramCount) {
      int len = qstrlen(typeNames[paramCount]);
      if (len <= 0)
        break;
      sig.append(typeNames[paramCount], len);
      sig.append(',');
    }
    if (paramCount == 0)
      sig.append(')');  // no parameters
    else
      sig[sig.size() - 1] = ')';
    sig.append('\0');

    int idx = metaObj.indexOfConstructor(sig.constData());
    if (idx < 0) {
      QByteArray norm = QMetaObject::normalizedSignature(sig.constData());
      idx = metaObj.indexOfConstructor(norm.constData());
    }
    if (idx < 0)
      return 0;

    void* returnValue = 0;
    void* param[] = {&returnValue, val0.data(), val1.data(), val2.data(), val3.data(), val4.data(),
                     val5.data(),  val6.data(), val7.data(), val8.data(), val9.data()};

    if (metaObj.static_metacall(QMetaObject::Call::CreateInstance, idx, param) >= 0)
      return 0;
    return returnValue;
  }
}

QVariant QObjectHelper::read(QObject* obj, const QMetaProperty& prop) {
  return prop.read(obj);
}

void QObjectHelper::write(QObject* obj, const QMetaProperty& prop, const QVariant& value) {
  prop.write(obj, value);
}

}  // namespace core

