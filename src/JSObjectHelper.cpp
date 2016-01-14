#include <node_buffer.h>
#include <sstream>
#include <QThread>
#include <QMetaProperty>
#include <QObject>
#include <QCoreApplication>
#include <QCache>
#include <QMultiHash>

#include "JSObjectHelper.h"
#include "ObjectTemplateStore.h"
#include "ObjectStore.h"
#include "Helper.h"
#include "Window.h"
#include "core/PrototypeStore.h"
#include "core/v8adapter.h"
#include "core/CommandArgument.h"
#include "core/qdeclare_metatype.h"
#include "core/QVariantArgument.h"
#include "core/Util.h"

using core::QVariantArgument;
using core::PrototypeStore;
using core::Util;

using v8::UniquePersistent;
using v8::ObjectTemplate;
using v8::EscapableHandleScope;
using v8::Local;
using v8::String;
using v8::PropertyCallbackInfo;
using v8::Value;
using v8::Isolate;
using v8::Array;
using v8::Object;
using v8::MaybeLocal;
using v8::Maybe;
using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Boolean;
using v8::Function;
using v8::Null;
using v8::FunctionTemplate;

QCache<const QMetaObject*, QMultiHash<QString, std::pair<int, ParameterTypes>>>
    JSObjectHelper::s_classMethodCache;

namespace {
Local<Object> toV8Object( Isolate* isolate, const CommandArgument args) {
  Local<Object> argsObj = Object::New(isolate);
  for (const auto& pair : args) {
    argsObj->Set(String::NewFromUtf8(isolate, pair.first.c_str()),
                 String::NewFromUtf8(isolate, pair.second.c_str()));
  }
  return argsObj;
}

QByteArray parameterTypeSignature(const QByteArray& methodSignature) {
  return methodSignature.mid(std::max(0, methodSignature.indexOf('(')));
}

bool qObjectPointerTypeCheck(QVariant var, const QByteArray& typeName) {
  if (var.isNull())
    return true;

  return var.canConvert<QObject*>() &&
         var.value<QObject*>()->inherits(typeName.left(typeName.size() - 1));
}

bool enumTypeCheck(QVariant var, const QByteArray& typeName) {
  if (!var.canConvert<int>()) {
    return false;
  }

  int typeId = QMetaType::type(typeName);
  if (typeId != QMetaType::UnknownType) {
    const QMetaObject* metaObj = QMetaType(typeId).metaObject();
    const QByteArray& enumName = Util::stipNamespace(typeName);
    return metaObj->indexOfEnumerator(enumName) >= 0;
  }

  return false;
}
}

void JSObjectHelper::invokeMethod(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  //  const QString& funcName = toQString(args.Callee()->GetName()->ToString());
  //  qDebug() << "invoking" << funcName;

  QObject* obj = ObjectStore::unwrap(args.Holder());
  if (!obj) {
    isolate->ThrowException(
        Exception::TypeError(String::NewFromUtf8(isolate, "can't convert to QObject")));
    return;
  }

  Q_ASSERT(obj->thread() == QThread::currentThread());

  // convert args to QVariantList
  QVariantList varArgs;
  for (int i = 0; i < args.Length(); i++) {
    varArgs.append(JSObjectHelper::toVariant(isolate, args[i]));
  }

  try {
    QVariant result = invokeMethodInternal(
        isolate, obj, toQString(args.Callee()->GetName()->ToString()), varArgs);
    if (result.isValid()) {
      args.GetReturnValue().Set(toV8Value(isolate, result));
    }
  } catch (const std::exception& e) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, e.what())));
  } catch (...) {
    qCritical() << "unexpected exception occured";
  }
}

QVariant JSObjectHelper::invokeMethodInternal(Isolate* isolate,
                                              QObject* object,
                                              const QString& methodName,
                                              QVariantList args) {
  if (args.size() > Q_METAMETHOD_INVOKE_MAX_ARGS) {
    std::stringstream ss;
    ss << "Can't invoke" << methodName.toUtf8().constData() << "with more than"
       << Q_METAMETHOD_INVOKE_MAX_ARGS << "arguments. args:" << args.size();
    throw std::runtime_error(ss.str());
  }

  if (!object) {
    throw std::runtime_error("object is null");
  }

  int methodIndex = -1;
  const QMetaObject* metaObj = object->metaObject();
  if (!s_classMethodCache.contains(metaObj)) {
    cacheMethods(metaObj);
  }

  // Find an appropriate method with the provided arguments
  for (MethodInfo methodInfo : s_classMethodCache[metaObj]->values(methodName)) {
    ParameterTypes parameterTypes = methodInfo.second;
    if (matchTypes(parameterTypes, args)) {
      // overwrite QVariant type with parameter type to match the method signature.
      // e.g. convert QLabel* to QWidget*
      for (int j = 0; j < parameterTypes.size(); j++) {
        args[j] = QVariant(QMetaType::type(parameterTypes[j]), args[j].data());
      }

      methodIndex = methodInfo.first;
      break;
    }
  }

  if (methodIndex == -1) {
    throw std::runtime_error("invalid arguments");
  }

  QMetaMethod method = object->metaObject()->method(methodIndex);

  if (!method.isValid()) {
    std::stringstream ss;
    ss << "Invalid method. name:" << method.name().constData() << "index:" << methodIndex;
    throw std::runtime_error(ss.str());
  } else if (method.access() != QMetaMethod::Public) {
    std::stringstream ss;
    ss << "Can't invoke non-public method. name:" << method.name().constData()
       << "index:" << methodIndex;
    throw std::runtime_error(ss.str());
  } else if (args.size() > method.parameterCount()) {
    qWarning() << "# of arguments is more than # of parameters. name:" << method.name()
               << "args size:" << args.size() << ",parameters size:" << method.parameterCount();
  }

  v8::HandleScope scope(isolate);

  QVariantArgument varArgs[Q_METAMETHOD_INVOKE_MAX_ARGS];
  for (int i = 0; i < args.size(); i++) {
    varArgs[i].value = args[i];
  }

  // Init return value
  QVariant returnValue;
  // If we pass nullptr as QVariant data, the function with QList<Window*> return type crashes.
  if (method.returnType() == qMetaTypeId<QList<Window*>>()) {
    returnValue = QVariant::fromValue(QList<Window*>());
  } else if (method.returnType() != qMetaTypeId<QVariant>() &&
             method.returnType() != qMetaTypeId<void>()) {
    returnValue = QVariant(method.returnType(), nullptr);
  }

  QGenericReturnArgument returnArg;
  if (returnValue.isValid()) {
    returnArg = QGenericReturnArgument(method.typeName(), returnValue.data());
  }

  Q_ASSERT(object->thread() == QCoreApplication::instance()->thread());
  bool result = method.invoke(object, Qt::DirectConnection, returnArg, varArgs[0], varArgs[1],
                              varArgs[2], varArgs[3], varArgs[4], varArgs[5], varArgs[6],
                              varArgs[7], varArgs[8], varArgs[9]);
  if (!result) {
    std::stringstream ss;
    ss << "invoking" << method.name().constData() << "failed";
    throw std::runtime_error(ss.str());
  }

  return returnValue;
}

Local<Value> JSObjectHelper::toV8ObjectFrom(Isolate* isolate, QObject* sourceObj) {
  if (!sourceObj) {
    return v8::Null(isolate);
  }

  if (const auto& maybeExistingObj = ObjectStore::singleton().find(sourceObj)) {
    return *maybeExistingObj;
  } else {
    const QMetaObject* metaObj = sourceObj->metaObject();

    Local<ObjectTemplate> objTempl =
        ObjectTemplateStore::singleton().getObjectTemplate(metaObj, isolate);

    // create a new object and store it
    MaybeLocal<Object> maybeObj = objTempl->NewInstance(isolate->GetCurrentContext());
    if (maybeObj.IsEmpty()) {
      qWarning() << "Failed to create an object";
      return v8::Null(isolate);
    }
    Local<Object> obj = maybeObj.ToLocalChecked();
    Maybe<bool> result = obj->SetPrototype(
        isolate->GetCurrentContext(),
        PrototypeStore::singleton().getOrCreatePrototype(metaObj, invokeMethod, isolate));
    ObjectStore::singleton().wrapAndInsert(sourceObj, obj);

    if (result.IsNothing()) {
      qWarning() << "setting prototype failed";
    }
    return obj;
  }
}

Local<Value> JSObjectHelper::toV8Value(Isolate* isolate, const QVariant& var) {
  if (var.canConvert<QObject*>()) {
    return toV8ObjectFrom(isolate, var.value<QObject*>());
  } else if (var.canConvert<CommandArgument>()) {
    return toV8Object(isolate, var.value<CommandArgument>());
  } else if (var.canConvert<std::string>()) {
    return toV8String(isolate, var.value<std::string>());
  }

  switch (var.type()) {
    case QVariant::Bool:
      return Boolean::New(isolate, var.toBool());
    case QVariant::Int:
      return v8::Int32::New(isolate, var.toInt());
    case QVariant::UInt:
      return v8::Uint32::New(isolate, var.toUInt());
    case QVariant::Double:
      return v8::Number::New(isolate, var.toDouble());
    case QVariant::ByteArray: {
      MaybeLocal<Object> maybeBuffer =
          node::Buffer::New(isolate, var.toByteArray().data(), var.toByteArray().size());
      if (maybeBuffer.IsEmpty()) {
        qWarning() << "Failed to create buffer object";
        return v8::Undefined(isolate);
      } else {
        return maybeBuffer.ToLocalChecked();
      }
    }
    case QVariant::String:
      return toV8String(isolate, var.toString());
    case QVariant::StringList: {
      const QStringList& strList = var.toStringList();
      Local<Array> array = Array::New(isolate, strList.size());
      for (int i = 0; i < strList.size(); i++) {
        array->Set(i, toV8String(isolate, strList[i]));
      }
      return array;
    }
    default:
      qWarning() << "can't convert" << var.typeName() << "to Local<Value>";
      return v8::Undefined(isolate);
  }
}

QVariant JSObjectHelper::toVariant(Isolate* isolate, v8::Local<v8::Value> value) {
  if (value->IsBoolean()) {
    return QVariant::fromValue(value->ToBoolean()->Value());
  } else if (value->IsInt32()) {
    return QVariant::fromValue(value->ToInt32()->Value());
  } else if (value->IsUint32()) {
    return QVariant::fromValue(value->ToUint32()->Value());
  } else if (value->IsNumber()) {
    return QVariant::fromValue(value->ToNumber()->Value());
  } else if (value->IsString()) {
    return QVariant::fromValue(toQString(value->ToString()));
  } else if (value->IsNull() || value->IsUndefined()) {
    return QVariant();
  } else if (value->IsArray()) {
    QVariantList list;
    Local<Array> arr = Local<Array>::Cast(value);
    for (uint32_t i = 0; i < arr->Length(); i++) {
      list.append(toVariant(isolate, arr->Get(i)));
    }
    return QVariant::fromValue(list);
  } else if (value->IsObject()) {
    QVariantMap map;
    Local<Object> obj = value->ToObject();

    if (obj->InternalFieldCount() > 0) {
      QObject* qObj = ObjectStore::unwrap(obj);
      if (qObj) {
        return QVariant::fromValue(qObj);
      } else {
        qWarning() << "object has internal field but it's null";
        return QVariant();
      }
    } else {
      Local<Array> keys = obj->GetOwnPropertyNames();
      for (uint32_t i = 0; i < keys->Length(); i++) {
        MaybeLocal<Value> maybeKey = keys->Get(isolate->GetCurrentContext(), i);
        if (maybeKey.IsEmpty()) {
          qWarning() << "key is empty";
          continue;
        }
        Local<Value> key = maybeKey.ToLocalChecked();
        if (!key->IsString()) {
          qWarning() << "key is not string";
          continue;
        }
        MaybeLocal<Value> maybeValue = obj->Get(isolate->GetCurrentContext(), key->ToString());
        if (maybeValue.IsEmpty()) {
          qWarning() << "value is empty";
          continue;
        }

        Local<Value> value = maybeValue.ToLocalChecked();
        if (!key->IsString()) {
          qWarning() << "value is not string";
          continue;
        }
        map.insert(toQString(key->ToString()), toVariant(isolate, value));
      }
      return QVariant::fromValue(map);
    }
  } else {
    qWarning() << "can't convert to QVariant";
    return QVariant();
  }
}

bool JSObjectHelper::matchTypes(QList<QByteArray> types, QVariantList args) {
  if (types.size() != args.size()) {
    return false;
  }

  for (int i = 0; i < types.size(); i++) {
    if (QMetaType::type(types[i]) != QMetaType::type(args[i].typeName()) &&
        !qObjectPointerTypeCheck(args[i], types[i]) && !enumTypeCheck(args[i], types[i])) {
      return false;
    }
  }

  return true;
}

void JSObjectHelper::connect(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);

  if (args.Length() <= 0 || !args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "invalid argument")));
    return;
  }

  QObject* obj = ObjectStore::unwrap(args.Holder());
  if (!obj) {
    isolate->ThrowException(
        Exception::TypeError(String::NewFromUtf8(isolate, "can't convert to QObject")));
    return;
  }

  v8::String::Utf8Value eventNameValue(
      args[0]->ToString(isolate->GetCurrentContext()).ToLocalChecked());
  const char* eventName = *eventNameValue;

  const QMetaObject* metaObj = obj->metaObject();
  int index = -1;
  for (int i = 0; i < metaObj->methodCount(); i++) {
    if (metaObj->method(i).methodType() == QMetaMethod::Signal &&
        metaObj->method(i).name() == eventName) {
      index = i;
      break;
    }
  }

  if (index < 0) {
    std::stringstream ss;
    ss << eventName << " is not defined";
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, ss.str().c_str())));
    return;
  }

  const QMetaMethod& method = metaObj->method(index);
  if (method.name() == "destoryed") {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "add listener to destroyed event is not allowed")));
    return;
  }

  //  qDebug() << method.methodSignature();
  QByteArray emitSignalSignature =
      parameterTypeSignature(method.methodSignature()).prepend("emitSignal");
  //  qDebug() << emitSignalSignature;
  const QMetaMethod& emitSignal =
      Helper::staticMetaObject.method(Helper::staticMetaObject.indexOfMethod(emitSignalSignature));
  if (emitSignal.isValid()) {
    QObject::connect(obj, method, &Helper::singleton(), emitSignal, Qt::UniqueConnection);
  } else {
    std::stringstream ss;
    ss << "parameter signature" << emitSignalSignature.constData() << "not supported";
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, ss.str().c_str())));
    return;
  }
}

void JSObjectHelper::cacheMethods(const QMetaObject* metaObj) {
  QMultiHash<QString, MethodInfo>* methodNameParameterTypesHash =
      new QMultiHash<QString, MethodInfo>();
  for (int i = 0; i < metaObj->methodCount(); i++) {
    const auto& method = metaObj->method(i);
    const QString& name = QString::fromLatin1(method.name());
    methodNameParameterTypesHash->insert(name, std::make_pair(i, method.parameterTypes()));
  }
  s_classMethodCache.insert(metaObj, methodNameParameterTypesHash);
}
