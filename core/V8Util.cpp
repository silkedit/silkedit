#include <vendor/node/src/node_buffer.h>
#include <sstream>
#include <QDebug>
#include <QLoggingCategory>

#include "V8Util.h"
#include "ObjectStore.h"
#include "qdeclare_metatype.h"
#include "ConstructorStore.h"
#include "FunctionInfo.h"
#include "QObjectUtil.h"
#include "JSValue.h"
#include "TextCursor.h"
#include "TextBlock.h"
#include "TextOption.h"
#include "Rect.h"
#include "QAbstractItemViewWrap.h"
#include "ModelIndex.h"
#include "QSizeWrap.h"
#include "QScrollBarWrap.h"
#include "ItemSelectionModel.h"

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
using v8::TryCatch;

namespace core {

v8::Persistent<v8::String> V8Util::s_hiddenQObjectKey;
v8::Persistent<v8::String> V8Util::s_constructorKey;

v8::Local<v8::String> V8Util::hiddenQObjectKey(Isolate* isolate) {
  if (s_hiddenQObjectKey.IsEmpty()) {
    s_hiddenQObjectKey.Reset(
        isolate,
        String::NewFromUtf8(isolate, "sourceObj", v8::NewStringType::kNormal).ToLocalChecked());
  }
  return s_hiddenQObjectKey.Get(isolate);
}

v8::Local<v8::String> V8Util::constructorKey(Isolate* isolate) {
  if (s_constructorKey.IsEmpty()) {
    s_constructorKey.Reset(
        isolate,
        String::NewFromUtf8(isolate, "constructor", v8::NewStringType::kNormal).ToLocalChecked());
  }
  return s_constructorKey.Get(isolate);
}

QVariant V8Util::toVariant(v8::Isolate* isolate, v8::Local<v8::Value> value) {
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
  } else if (value->IsFunction()) {
    return QVariant::fromValue(FunctionInfo{isolate, Local<Function>::Cast(value)});
  } else if (value->IsObject()) {
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
      return toVariantMap(isolate, obj);
    }
  } else {
    qWarning() << "can't convert to QVariant";
    return QVariant();
  }
}

bool V8Util::isEnum(QVariant var) {
  if (!var.canConvert<int>()) {
    return false;
  }

  int typeId = QMetaType::type(var.typeName());
  if (typeId != QMetaType::UnknownType) {
    return QMetaType::typeFlags(typeId).testFlag(QMetaType::IsEnumeration);
  }

  return false;
}

v8::Local<v8::Value> V8Util::toV8Value(v8::Isolate* isolate, const QVariant& var) {
  auto typeId = QMetaType::type(var.typeName());
  // don't use QVariant::canConvert for Wrapper type
  // When var is Window*, canConvert<QAbstractItemView*>() returns true.
  // if (var.canConvert<QAbstractItemView*>()) {
  if (typeId == qMetaTypeId<QAbstractItemView*>()) {
    return toV8ObjectFrom(isolate, new QAbstractItemViewWrap(var.value<QAbstractItemView*>()));
  } else if (typeId == qMetaTypeId<QItemSelectionModel*>()) {
    return toV8ObjectFrom(isolate, new ItemSelectionModel(var.value<QItemSelectionModel*>()));
  } else if (typeId == qMetaTypeId<QScrollBar*>()) {
    return toV8ObjectFrom(isolate, new QScrollBarWrap(var.value<QScrollBar*>()));
  } else if (var.canConvert<QObject*>()) {
    return toV8ObjectFrom(isolate, var.value<QObject*>());
  } else if (var.canConvert<QModelIndex>()) {
    return toV8ObjectFrom(isolate, new ModelIndex(var.value<QModelIndex>()));
  } else if (var.canConvert<QRect>()) {
    return toV8ObjectFrom(isolate, new Rect(var.value<QRect>()));
  } else if (var.canConvert<QSize>()) {
    return toV8ObjectFrom(isolate, new QSizeWrap(var.value<QSize>()));
  } else if (var.canConvert<QTextBlock>()) {
    return toV8ObjectFrom(isolate, new TextBlock(var.value<QTextBlock>()));
  } else if (var.canConvert<QTextCursor>()) {
    return toV8ObjectFrom(isolate, new TextCursor(var.value<QTextCursor>()));
  } else if (var.canConvert<QTextOption>()) {
    return toV8ObjectFrom(isolate, new TextOption(var.value<QTextOption>()));
  } else if (var.canConvert<CommandArgument>()) {
    return toV8Object(isolate, var.value<CommandArgument>());
  } else if (var.canConvert<std::string>()) {
    return toV8String(isolate, var.value<std::string>());
  } else if (var.canConvert<QVariantList>()) {
    QSequentialIterable iterable = var.value<QSequentialIterable>();
    Local<Array> array = Array::New(isolate, iterable.size());
    int i = 0;
    for (const QVariant& v : iterable) {
      array->Set(i++, toV8Value(isolate, v));
    }
    return array;
  } else if (var.canConvert<JSNull>()) {
    return v8::Null(isolate);
  } else if (isEnum(var)) {
    return v8::Int32::New(isolate, var.toInt());
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

v8::Local<v8::Object> V8Util::toV8Object(v8::Isolate* isolate, const CommandArgument args) {
  Local<Object> argsObj = Object::New(isolate);
  for (const auto& pair : args) {
    argsObj->Set(String::NewFromUtf8(isolate, pair.first.c_str()), toV8Value(isolate, pair.second));
  }
  return argsObj;
}

v8::MaybeLocal<v8::Object> V8Util::newInstance(v8::Isolate* isolate,
                                               v8::Local<v8::Function> constructor,
                                               void* sourceObj) {
  EscapableHandleScope scope(isolate);
  Q_ASSERT(!constructor.IsEmpty());
  Local<ObjectTemplate> objTempl = ObjectTemplate::New(isolate);
  objTempl->SetInternalFieldCount(1);
  Local<Object> wrappedObj = objTempl->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
  wrappedObj->SetAlignedPointerInInternalField(0, sourceObj);
  Local<String> hiddenKey = hiddenQObjectKey(isolate);

  // set sourceObj as hidden value to tell constructor set it as internal object
  constructor->SetHiddenValue(hiddenKey, wrappedObj);
  MaybeLocal<Object> maybeObj = constructor->NewInstance(isolate->GetCurrentContext(), 0, nullptr);
  constructor->DeleteHiddenValue(hiddenKey);
  if (maybeObj.IsEmpty()) {
    qWarning() << "Failed to create an object";
    return v8::MaybeLocal<Object>();
  }
  Local<Object> obj = maybeObj.ToLocalChecked();

  // set constructor
  Maybe<bool> result = obj->Set(isolate->GetCurrentContext(), constructorKey(isolate), constructor);
  if (result.IsNothing() || !result.FromJust()) {
    qWarning() << "failed to set constructor property";
    return v8::MaybeLocal<Object>();
  }

  return scope.Escape(obj);
}

v8::Local<v8::Value> V8Util::toV8ObjectFrom(v8::Isolate* isolate, QObject* sourceObj) {
  if (!sourceObj) {
    return v8::Null(isolate);
  }

  if (const auto& maybeExistingObj = ObjectStore::singleton().find(sourceObj, isolate)) {
    return *maybeExistingObj;
  } else {
    const QMetaObject* metaObj = sourceObj->metaObject();
    Local<Function> ctor = ConstructorStore::singleton().findOrCreateConstructor(metaObj, isolate);

    MaybeLocal<Object> maybeObj = newInstance(isolate, ctor, sourceObj);
    if (maybeObj.IsEmpty()) {
      return v8::Null(isolate);
    }

    auto obj = maybeObj.ToLocalChecked();
    ObjectStore::singleton().wrapAndInsert(sourceObj, obj, isolate);
    return obj;
  }
}

QVariantMap V8Util::toVariantMap(v8::Isolate* isolate, v8::Local<v8::Object> obj) {
  QVariantMap map;
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
    map.insert(toQString(key->ToString()), toVariant(isolate, value));
  }
  return map;
}

void V8Util::throwError(v8::Isolate* isolate, const std::string& msg) {
  throwError(isolate, msg.c_str());
}

void V8Util::throwError(v8::Isolate* isolate, const char* msg) {
  isolate->ThrowException(v8::Exception::Error(
      v8::String::NewFromUtf8(isolate, msg, v8::NewStringType::kNormal).ToLocalChecked()));
}

void V8Util::invokeQObjectMethod(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  //  const QString& funcName = toQString(args.Callee()->GetName()->ToString());
  //  qDebug() << "invoking" << funcName;

  QObject* obj = ObjectStore::unwrap(args.Holder());
  if (!obj) {
    throwError(isolate, "failed to get QObject. maybe caller is not QObject.");
    return;
  }

  // convert args to QVariantList
  QVariantList varArgs;
  for (int i = 0; i < args.Length(); i++) {
    varArgs.append(toVariant(isolate, args[i]));
  }

  try {
    QVariant result = QObjectUtil::invokeQObjectMethodInternal(
        obj, toQString(args.Callee()->GetName()->ToString()), varArgs);
    if (result.isValid()) {
      args.GetReturnValue().Set(toV8Value(isolate, result));
    }
  } catch (const std::exception& e) {
    V8Util::throwError(isolate, e.what());
  } catch (...) {
    qCritical() << "unexpected exception occured";
  }
}

void V8Util::emitQObjectSignal(const v8::FunctionCallbackInfo<v8::Value>& args) {
  //  qDebug() << "emitQObjectSignal";

  Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);

  if (args.Length() <= 0 || !args[0]->IsString()) {
    throwError(isolate, "invalid argument");
    return;
  }

  QObject* obj = ObjectStore::unwrap(args.Holder());
  if (!obj) {
    throwError(isolate, "can't convert to QObject");
    return;
  }

  v8::String::Utf8Value eventNameValue(
      args[0]->ToString(isolate->GetCurrentContext()).ToLocalChecked());
  const char* eventName = *eventNameValue;

  // convert args to QVariantList
  QVariantList varArgs;
  // skip args[0] because it is the event name
  for (int i = 1; i < args.Length(); i++) {
    varArgs.append(toVariant(isolate, args[i]));
  }

  try {
    QVariant result = QObjectUtil::invokeQObjectMethodInternal(obj, eventName, varArgs);
    if (result.isValid()) {
      args.GetReturnValue().Set(toV8Value(isolate, result));
    }
  } catch (const std::exception& e) {
    V8Util::throwError(isolate, e.what());
  } catch (...) {
    qCritical() << "unexpected exception occured";
  }
}

QString V8Util::getErrorMessage(Isolate* isolate, const TryCatch& trycatch) {
  if (trycatch.HasCaught()) {
    MaybeLocal<Value> maybeStackTrace = trycatch.StackTrace(isolate->GetCurrentContext());
    Local<Value> exception = trycatch.Exception();
    std::stringstream ss;
    if (!maybeStackTrace.IsEmpty()) {
      auto str = maybeStackTrace.ToLocalChecked();
      if (str->IsString()) {
        String::Utf8Value stackTraceStr(str);
        ss << *stackTraceStr;
      }
    } else if (!exception.IsEmpty() && exception->IsString()) {
      String::Utf8Value exceptionStr(exception);
      ss << "error: " << *exceptionStr;
    } else {
      qWarning() << "Can't get an error message from an exception";
      return "";
    }

    return ss.str().c_str();
  }

  return "";
}

QVariant V8Util::callJSFunc(v8::Isolate* isolate,
                            v8::Local<v8::Function> fn,
                            Local<Value> recv,
                            int argc,
                            Local<Value> argv[]) {
  TryCatch trycatch(isolate);
  MaybeLocal<Value> maybeResult = fn->Call(isolate->GetCurrentContext(), recv, argc, argv);
  if (trycatch.HasCaught()) {
    QLoggingCategory category("silkedit");
    qCCritical(category) << getErrorMessage(isolate, trycatch);

    return QVariant();
  } else if (maybeResult.IsEmpty()) {
    QLoggingCategory category("silkedit");
    qCCritical(category) << "maybeResult is empty (but exception is not thrown...)";
    return QVariant();
  }

  return V8Util::toVariant(isolate, maybeResult.ToLocalChecked());
}

bool V8Util::checkArguments(const v8::FunctionCallbackInfo<v8::Value> args,
                            int numArgs,
                            std::function<bool()> validateFn) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != numArgs) {
    std::stringstream ss;
    ss << "arguments size mismatched. expected:" << numArgs << " actual:" << args.Length();
    V8Util::throwError(isolate, ss.str());
    return false;
  }

  if (!validateFn()) {
    V8Util::throwError(isolate, "invalid argument");
    return false;
  }

  return true;
}

}  // namespace core
