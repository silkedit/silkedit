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
#include "MessageBox.h"
#include "KeymapManager.h"
#include "CommandManager.h"
#include "App.h"
#include "Window.h"
#include "ConfigDialog.h"
#include "DocumentManager.h"
#include "ProjectManager.h"
#include "FileDialog.h"
#include "core/v8adapter.h"
#include "core/macros.h"
#include "core/Config.h"
#include "core/Constants.h"
#include "core/QVariantArgument.h"
#include "core/Condition.h"
#include "core/ConditionManager.h"
#include "bridge/JSStaticObject.h"

using core::Config;
using core::Constants;
using core::PrototypeStore;
using core::QVariantArgument;
using core::Condition;
using core::ConditionManager;

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
using v8::FunctionCallbackInfo;
using v8::Function;

namespace {

bool checkArguments(const v8::FunctionCallbackInfo<v8::Value> args,
                    int numArgs,
                    std::function<bool()> validateFn) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != numArgs) {
    std::stringstream ss;
    ss << "arguments size mismatched. expected:" << numArgs << " actual:" << args.Length();
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, ss.str().data())));
    return false;
  }

  if (!validateFn()) {
    isolate->ThrowException(
        Exception::TypeError(String::NewFromUtf8(isolate, "invalid arguments")));
    return false;
  }

  return true;
}

/*
  Window static methods
*/

void loadMenu(const FunctionCallbackInfo<Value>& args) {
  if (!checkArguments(args, 2, [&] { return args[0]->IsString() && args[1]->IsString(); })) {
    return;
  }

  Window::loadMenu(toQString(args[0]->ToString()), toQString(args[1]->ToString()));
}

void loadToolbar(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (!checkArguments(args, 2, [&] { return args[0]->IsString() && args[1]->IsString(); })) {
    return;
  }

  Window::loadToolbar(toQString(args[0]->ToString()), toQString(args[1]->ToString()));
}

/*
  ConfigDialog static methods
*/

void loadConfigDefinition(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (!checkArguments(args, 2, [&] { return args[0]->IsString() && args[1]->IsString(); })) {
    return;
  }

  ConfigDialog::loadDefinition(toQString(args[0]->ToString()), toQString(args[1]->ToString()));
}

/*
  Condition static methods
*/
void check(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (!checkArguments(args, 3, [&] {
        return args[0]->IsString() && args[1]->IsInt32() && args[2]->IsString();
      })) {
    return;
  }

  bool result = Condition::check(toQString(args[0]->ToString()),
                                 static_cast<Condition::Operator>(args[1]->ToInt32()->Value()),
                                 toQString(args[2]->ToString()));
  args.GetReturnValue().Set(Boolean::New(args.GetIsolate(), result));
}
}

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
  // NOTE: staticMetaObject.className() inclues namespace, so don't use it as class name
  Util::stipNamespace(KeymapManager::staticMetaObject.className());
  setSingletonObj(exports, &API::singleton(), "API");
  setSingletonObj(exports, App::instance(), Util::stipNamespace(App::staticMetaObject.className()));
  setSingletonObj(exports, &Config::singleton(),
                  Util::stipNamespace(Config::staticMetaObject.className()));
  setSingletonObj(exports, &Constants::singleton(),
                  Util::stipNamespace(Constants::staticMetaObject.className()));
  setSingletonObj(exports, &KeymapManager::singleton(),
                  Util::stipNamespace(KeymapManager::staticMetaObject.className()));
  setSingletonObj(exports, &CommandManager::singleton(),
                  Util::stipNamespace(CommandManager::staticMetaObject.className()));
  setSingletonObj(exports, &DocumentManager::singleton(),
                  Util::stipNamespace(DocumentManager::staticMetaObject.className()));
  setSingletonObj(exports, &ProjectManager::singleton(),
                  Util::stipNamespace(ProjectManager::staticMetaObject.className()));
  // ConditionManager::add accepts JS object as argument, so we can't use setSingletonObj (this
  // converts JS object to QObject* or QVariantMap internally)
  ConditionManager::Init(exports);

  // init classes
  registerClass<Condition>(exports);
  registerClass<ConfigDialog>(exports);
  registerClass<Dialog>(exports);
  registerClass<DialogButtonBox>(exports);
  registerClass<FileDialog>(exports);
  registerClass<Label>(exports);
  registerClass<LineEdit>(exports);
  registerClass<MessageBox>(exports);
  registerClass<Window>(exports);
  registerClass<VBoxLayout>(exports);
}

template <typename T>
void bridge::Handler::registerClass(v8::Local<v8::Object> exports) {
  auto ctor = JSStaticObject<T>::Init(exports);
  registerStaticMethods(T::staticMetaObject, ctor);
}

void bridge::Handler::registerStaticMethods(const QMetaObject& metaObj,
                                            v8::Local<v8::Function> ctor) {
  if (metaObj.className() == Window::staticMetaObject.className()) {
    NODE_SET_METHOD(ctor, "loadMenu", loadMenu);
    NODE_SET_METHOD(ctor, "loadToolbar", loadToolbar);
  } else if (metaObj.className() == ConfigDialog::staticMetaObject.className()) {
    NODE_SET_METHOD(ctor, "loadDefinition", loadConfigDefinition);
  } else if (metaObj.className() == Condition::staticMetaObject.className()) {
    NODE_SET_METHOD(ctor, "check", check);
  }
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
      metaObj, JSObjectHelper::invokeMethod, isolate, true));

  Maybe<bool> result =
      exports->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, name), obj);
  if (result.IsNothing()) {
    qWarning() << "setting exports failed";
  }
}

// register builtin silkeditbridge module
NODE_MODULE_CONTEXT_AWARE_BUILTIN(silkeditbridge, bridge::Handler::init)
