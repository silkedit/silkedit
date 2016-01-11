#include <node.h>
#include <algorithm>
#include <memory>
#include <QDebug>
#include <QVariant>
#include <QMetaObject>
#include <QMetaMethod>
#include <QThread>
#include <QCoreApplication>

#include "Handler.h"
#include "JSHandler.h"
#include "core/PrototypeStore.h"
#include "ObjectTemplateStore.h"
#include "API.h"
#include "JSObjectHelper.h"
#include "Dialog.h"
#include "VBoxLayout.h"
#include "DialogButtonBox.h"
#include "LineEdit.h"
#include "Label.h"
#include "KeymapManager.h"
#include "core/v8adapter.h"
#include "core/macros.h"
#include "core/Config.h"
#include "core/Constants.h"
#include "core/QVariantArgument.h"
#include "bridge/JSStaticObject.h"

using core::Config;
using core::Constants;
using core::PrototypeStore;
using core::QVariantArgument;

using v8::Array;
using v8::Boolean;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Value;
using v8::External;
using v8::Exception;
using v8::Persistent;
using v8::EscapableHandleScope;
using v8::Maybe;
using v8::FunctionTemplate;
using v8::FunctionCallback;
using v8::Function;

void bridge::Handler::init(Local<Object> exports,
                           v8::Local<v8::Value>,
                           v8::Local<v8::Context> context,
                           void*) {
  Isolate* isolate = context->GetIsolate();
  HandleScope handle_scope(isolate);

  // init JSHandler singleton object
  Local<ObjectTemplate> jsHandler = ObjectTemplate::New(isolate);
  MaybeLocal<Object> maybeJsHandlerObj = jsHandler->NewInstance(context);
  if (maybeJsHandlerObj.IsEmpty()) {
    qWarning() << "Failed to create JSHandler object";
    return;
  }
  Local<Object> jsHandlerObj = maybeJsHandlerObj.ToLocalChecked();
  Maybe<bool> result =
      exports->Set(context, String::NewFromUtf8(isolate, "JSHandler"), jsHandlerObj);
  if (result.IsNothing()) {
    qWarning() << "exposing JSHandler failed";
  }
  JSHandler::init(jsHandlerObj);

  NODE_SET_METHOD(exports, "connect", JSObjectHelper::connect);
  NODE_SET_METHOD(exports, "lateInit", lateInit);

  // register enums in Qt namespace
  qRegisterMetaType<Qt::Orientation>();
  int id = QMetaType::type("Qt::Orientation");
  Q_ASSERT(id != QMetaType::UnknownType);
  const QMetaObject* metaObj = QMetaType::metaObjectForType(id);
  for (int i = 0; i < metaObj->enumeratorCount(); i++) {
    const QMetaEnum& metaEnum = metaObj->enumerator(i);
    v8::Local<v8::Object> enumObj = v8::Object::New(isolate);
    for (int j = 0; j < metaEnum.keyCount(); j++) {
      enumObj->Set(v8::String::NewFromUtf8(isolate, metaEnum.key(j)),
                   v8::Integer::New(isolate, metaEnum.value(j)));
    }
    Maybe<bool> result =
        exports->Set(context, v8::String::NewFromUtf8(isolate, metaEnum.name()), enumObj);
    if (result.IsNothing()) {
      qCritical() << "failed to set enum" << metaEnum.name();
    }
  }
}

void bridge::Handler::lateInit(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Q_ASSERT(args.Length() == 1);
  Q_ASSERT(args[0]->IsObject());

  Local<Object> exports = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
  // init singleton objects
  setSingletonObj(exports, &API::singleton(), "API");
  setSingletonObj(exports, &Config::singleton(), "Config");
  setSingletonObj(exports, &Constants::singleton(), "Constants");
  setSingletonObj(exports, &KeymapManager::singleton(), "KeymapManager");

  // init classes
  JSStaticObject<Dialog>::Init(exports);
  JSStaticObject<VBoxLayout>::Init(exports);
  JSStaticObject<DialogButtonBox>::Init(exports);
  JSStaticObject<LineEdit>::Init(exports);
  JSStaticObject<Label>::Init(exports);
}

void bridge::Handler::setSingletonObj(Local<Object>& exports,
                                      QObject* sourceObj,
                                      const char* name) {
  Isolate* isolate = exports->GetIsolate();
  const QMetaObject* metaObj = sourceObj->metaObject();
  Local<ObjectTemplate> objTempl =
      ObjectTemplateStore::singleton().createObjectTemplate(metaObj, isolate);
  MaybeLocal<Object> maybeObj = objTempl->NewInstance(isolate->GetCurrentContext());
  if (maybeObj.IsEmpty()) {
    qWarning() << "Failed to create an object";
    return;
  }

  Local<Object> obj = maybeObj.ToLocalChecked();
  obj->SetAlignedPointerInInternalField(0, sourceObj);

  // sets __proto__ (this doesn't create prototype property)
  obj->SetPrototype(PrototypeStore::singleton().getOrCreatePrototype(
      metaObj, JSObjectHelper::invokeMethod, isolate));

  Maybe<bool> result =
      exports->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, name), obj);
  if (result.IsNothing()) {
    qWarning() << "setting exports failed";
  }
}

// register builtin silkeditbridge module
NODE_MODULE_CONTEXT_AWARE_BUILTIN(silkeditbridge, bridge::Handler::init)
