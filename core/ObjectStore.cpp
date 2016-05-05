#include <QVariant>
#include <QDebug>
#include <QMetaMethod>
#include <QThread>
#include <QCoreApplication>

#include "ObjectStore.h"
#include "V8Util.h"
#include "JSHandler.h"

using v8::UniquePersistent;
using v8::Object;
using v8::MaybeLocal;
using v8::Local;
using v8::Function;
using v8::Value;
using v8::Isolate;
using v8::Null;
using v8::String;

namespace core {

std::unordered_map<QObject*, v8::UniquePersistent<v8::Object>> ObjectStore::s_objects;
std::unordered_set<QObject*> ObjectStore::s_destroyedConnectedObjects;

QObject* ObjectStore::unwrap(v8::Local<v8::Object> obj) {
  Q_ASSERT(!obj.IsEmpty());
  // This is the case when QObject method is called without QObject.
  // e.g.  new silkedit.App.activeTextEdit()
  if (obj->InternalFieldCount() == 0) {
    return nullptr;
  }
  // Cast to ObjectWrap before casting to T.  A direct cast from void
  // to T won't work right when T has more than one base class.
  void* ptr = obj->GetAlignedPointerFromInternalField(0);
  return static_cast<QObject*>(ptr);
}

void ObjectStore::removeDestroyedConnectedObject(QObject* destroyedObj) {
  // emit destroyed signal to JS side
  if (s_destroyedConnectedObjects.count(destroyedObj) != 0) {
    QVariantList args{QVariant::fromValue(destroyedObj)};
    auto isolate = Isolate::GetCurrent();
    if (isolate && !isolate->IsExecutionTerminating() && !isolate->IsDead()) {
      v8::Locker locker(isolate);
      v8::HandleScope handle_scope(isolate);
      JSHandler::emitSignal(isolate, destroyedObj, QStringLiteral("destroyed"), args);
      s_destroyedConnectedObjects.erase(destroyedObj);
    }
  }
}

void ObjectStore::wrapAndInsert(QObject* obj, v8::Local<v8::Object> jsObj, v8::Isolate* isolate) {
  Q_ASSERT(obj);
  Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
  // When registered QObject is destoryed, delete its associated JS object
  connect(obj, &QObject::destroyed, [=](QObject* destroyedObj) {
    if (!destroyedObj) {
      qWarning() << "destroyedObj is null";
      return;
    }

    removeDestroyedConnectedObject(destroyedObj);

    // emit destroyed signal to JS side
    if (s_destroyedConnectedObjects.count(destroyedObj) != 0) {
      QVariantList args{QVariant::fromValue(destroyedObj)};
      auto isolate = Isolate::GetCurrent();
      if (isolate && !isolate->IsExecutionTerminating() && !isolate->IsDead()) {
        v8::Locker locker(isolate);
        v8::HandleScope handle_scope(isolate);
        JSHandler::emitSignal(isolate, destroyedObj, QStringLiteral("destroyed"), args);
        s_destroyedConnectedObjects.erase(destroyedObj);
      }
    }

    if (s_objects.count(destroyedObj) != 0) {
      s_objects.at(destroyedObj).Reset();
      s_objects.erase(destroyedObj);
    }
  });

  jsObj->SetAlignedPointerInInternalField(0, obj);
  UniquePersistent<Object> persistentObj(isolate, jsObj);
  persistentObj.SetWeak(obj, WeakCallback);
  persistentObj.MarkIndependent();
  auto pair = std::make_pair(obj, std::move(persistentObj));
  s_objects.insert(std::move(pair));
}

void ObjectStore::registerDestroyedConnectedObject(QObject* obj) {
  s_destroyedConnectedObjects.insert(obj);
}

boost::optional<v8::Local<v8::Object>> ObjectStore::find(QObject* obj, v8::Isolate* isolate) {
  Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
  if (s_objects.count(obj) != 0) {
    return s_objects.at(obj).Get(isolate);
  } else {
    return boost::none;
  }
}

void ObjectStore::clearDestroyedConnectedObjects() {
  for (auto obj : s_destroyedConnectedObjects) {
    removeDestroyedConnectedObject(obj);
  }
}

void ObjectStore::WeakCallback(const v8::WeakCallbackData<v8::Object, QObject>& data) {
  Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
  qDebug() << "WeakCallback";
  v8::Isolate* isolate = data.GetIsolate();
  v8::HandleScope scope(isolate);
  QObject* wrap = data.GetParameter();
  if (s_objects.count(wrap) != 0) {
    s_objects.erase(wrap);
    const QVariant& state = wrap->property(OBJECT_STATE);
    if (state.isValid() && state.value<ObjectState>() == ObjectState::NewFromJS) {
      wrap->deleteLater();
      qDebug() << wrap->metaObject()->className() << "is deleted";
    }
  }
}

}  // namespace core
