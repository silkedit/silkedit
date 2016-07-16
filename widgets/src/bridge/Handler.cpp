#include <algorithm>
#include <memory>
#include <QDebug>
#include <QVariant>
#include <QMetaObject>
#include <QMetaMethod>
#include <QThread>
#include <QCoreApplication>

#include "Handler.h"
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
#include "TextEdit.h"
#include "JSStaticObject.h"
#include "WebView.h"
#include "WebPage.h"
#include "WebChannel.h"
#include "Console.h"
#include "FindReplaceView.h"
#include "util/YamlUtil.h"
#include "core/Font.h"
#include "core/JSHandler.h"
#include "core/V8Util.h"
#include "core/ObjectTemplateStore.h"
#include "core/macros.h"
#include "core/Config.h"
#include "core/QVariantArgument.h"
#include "core/ConditionManager.h"
#include "core/ObjectStore.h"
#include "core/KeyEvent.h"
#include "core/Event.h"
#include "core/TextCursor.h"
#include "core/TextBlock.h"
#include "core/MessageHandler.h"
#include "core/PackageManager.h"
#include "core/TextOption.h"
#include "core/Completer.h"
#include "core/StringListModel.h"
#include "core/Rect.h"
#include "core/ItemSelectionModel.h"
#include "core/QtEnums.h"
#include "core/Validator.h"
#include "core/atom/node_includes.h"

using core::Config;
using core::QVariantArgument;
using core::ConditionManager;
using core::Font;
using core::ObjectTemplateStore;
using core::JSHandler;
using core::ObjectStore;
using core::KeyEvent;
using core::Event;
using core::TextCursor;
using core::TextBlock;
using core::PackageManager;
using core::TextOption;
using core::Completer;
using core::StringListModel;
using core::Rect;
using core::ItemSelectionModel;
using core::QtEnums;
using core::Validator;

#ifdef Q_OS_WIN
// MessageBox is defined in winuser.h
#undef MessageBox
#endif
using view::MessageBox;

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

/*
  Window static methods
*/
void createWithNewFile(const FunctionCallbackInfo<Value>& args) {
  auto win = Window::createWithNewFile();
  args.GetReturnValue().Set(V8Util::toV8ObjectFrom(args.GetIsolate(), win));
}

void loadMenu(const FunctionCallbackInfo<Value>& args) {
  if (!V8Util::checkArguments(args, 2,
                              [&] { return args[0]->IsString() && args[1]->IsString(); })) {
    return;
  }

  Window::loadMenu(V8Util::toQString(args[0]->ToString()), V8Util::toQString(args[1]->ToString()));
}

void loadToolbar(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (!V8Util::checkArguments(args, 2,
                              [&] { return args[0]->IsString() && args[1]->IsString(); })) {
    return;
  }

  Window::loadToolbar(V8Util::toQString(args[0]->ToString()),
                      V8Util::toQString(args[1]->ToString()));
}

void windows(const v8::FunctionCallbackInfo<v8::Value>& args) {
  auto windows = Window::windows();

  Isolate* isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Array> array = Array::New(isolate, windows.size());
  for (int i = 0; i < windows.size(); i++) {
    array->Set(i, V8Util::toV8ObjectFrom(isolate, windows[i]));
  }
  args.GetReturnValue().Set(array);
}

/*
  ConfigDialog static methods
*/

void loadConfigDefinition(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (!V8Util::checkArguments(args, 2,
                              [&] { return args[0]->IsString() && args[1]->IsString(); })) {
    return;
  }

  ConfigDialog::loadDefinition(V8Util::toQString(args[0]->ToString()),
                               V8Util::toQString(args[1]->ToString()));
}
}

template <typename T>
void bridge::Handler::registerQtEnum(v8::Local<v8::Context> context,
                                     Local<Object> exports,
                                     Isolate* isolate,
                                     const char* name) {
  qRegisterMetaType<T>();
  int id = QMetaType::type(name);
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
  NODE_SET_METHOD(exports, "disconnect", JSObjectHelper::disconnect);
  NODE_SET_METHOD(exports, "emit", V8Util::emitQObjectSignal);
  NODE_SET_METHOD(exports, "lateInit", lateInit);
  NODE_SET_METHOD(exports, "info", info);
  NODE_SET_METHOD(exports, "warn", warn);
  NODE_SET_METHOD(exports, "error", error);
  NODE_SET_METHOD(exports, "translate", YamlUtil::translate);

  // register enums in Qt namespace
  registerQtEnum<Qt::CaseSensitivity>(context, exports, isolate, "Qt::CaseSensitivity");
  registerQtEnum<Qt::Orientation>(context, exports, isolate, "Qt::Orientation");
  registerQtEnum<Qt::Key>(context, exports, isolate, "Qt::Key");
}

void bridge::Handler::lateInit(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Q_ASSERT(args.Length() == 1);
  Q_ASSERT(args[0]->IsObject());

  Local<Object> exports = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
  // init singleton objects
  // NOTE: staticMetaObject.className() inclues namespace, so don't use it as class name
  setSingletonObj(exports, App::instance(),
                  Util::stripNamespace(App::staticMetaObject.className()));
  setSingletonObj(exports, &CommandManager::singleton(),
                  Util::stripNamespace(CommandManager::staticMetaObject.className()));
  setSingletonObj(exports, &DocumentManager::singleton(),
                  Util::stripNamespace(DocumentManager::staticMetaObject.className()));
  setSingletonObj(exports, &KeymapManager::singleton(),
                  Util::stripNamespace(KeymapManager::staticMetaObject.className()));
  setSingletonObj(exports, &ProjectManager::singleton(),
                  Util::stripNamespace(ProjectManager::staticMetaObject.className()));
  setSingletonObj(exports, &PackageManager::singleton(),
                  Util::stripNamespace(PackageManager::staticMetaObject.className()));

  // Config::get returns config whose type is decided based on ConfigDefinition, so we need to
  // handle it specially
  Config::Init(exports);

  // init classes for QObject subclasses
  registerClass<Completer>(exports);
  registerClass<ConfigDialog>(exports);
  registerClass<Console>(exports);
  registerClass<Dialog>(exports);
  registerClass<DialogButtonBox>(exports);
  registerClass<Event>(exports);
  registerClass<FileDialog>(exports);
  registerClass<FindReplaceView>(exports);
  registerClass<Label>(exports);
  registerClass<LineEdit>(exports);
  registerClass<view::MessageBox>(exports);
  registerClass<StringListModel>(exports);
  registerClass<TextEdit>(exports);
  registerClass<VBoxLayout>(exports);
  registerClass<WebChannel>(exports);
  registerClass<WebPage>(exports);
  registerClass<WebView>(exports);
  registerClass<Window>(exports);
  registerClass<Validator>(exports);

  // Wrappers
  registerClass<Font>(exports);
  registerClass<KeyEvent>(exports);
  registerClass<ItemSelectionModel>(exports);
  registerClass<Rect>(exports);
  registerClass<TextBlock>(exports);
  registerClass<TextCursor>(exports);
  registerClass<TextOption>(exports);

  // Qt Enums
  registerClass<QtEnums>(exports);

  // Condition::add accepts JS object as argument, so we can't use setSingletonObj (this
  // converts JS object to QObject* or QVariantMap internally)
  ConditionManager::Init(exports);
}

void bridge::Handler::info(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() > 0 && args[0]->IsString()) {
    Local<String> msg = args[0]->ToString(isolate->GetCurrentContext()).ToLocalChecked();
    QLoggingCategory category(SILKEDIT_CATEGORY);
    qCInfo(category).noquote() << V8Util::toQString(msg);
  }
}

void bridge::Handler::warn(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() > 0 && args[0]->IsString()) {
    Local<String> msg = args[0]->ToString(isolate->GetCurrentContext()).ToLocalChecked();
    QLoggingCategory category(SILKEDIT_CATEGORY);
    qCWarning(category).noquote() << V8Util::toQString(msg);
  }
}

void bridge::Handler::error(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() > 0 && args[0]->IsString()) {
    Local<String> msg = args[0]->ToString(isolate->GetCurrentContext()).ToLocalChecked();
    QLoggingCategory category(SILKEDIT_CATEGORY);
    qCCritical(category).noquote() << V8Util::toQString(msg);
  }
}

template <typename T>
void bridge::Handler::registerClass(v8::Local<v8::Object> exports) {
  auto ctor = bridge::JSStaticObject<T>::Init(exports);
  registerStaticMethods(T::staticMetaObject, ctor);
}

void bridge::Handler::registerStaticMethods(const QMetaObject& metaObj,
                                            v8::Local<v8::Function> ctor) {
  if (metaObj.className() == ConfigDialog::staticMetaObject.className()) {
    NODE_SET_METHOD(ctor.As<v8::Object>(), "loadDefinition", loadConfigDefinition);
  } else if (metaObj.className() == Window::staticMetaObject.className()) {
    NODE_SET_METHOD(ctor.As<v8::Object>(), "createWithNewFile", createWithNewFile);
    NODE_SET_METHOD(ctor.As<v8::Object>(), "loadMenu", loadMenu);
    NODE_SET_METHOD(ctor.As<v8::Object>(), "loadToolbar", loadToolbar);
    NODE_SET_METHOD(ctor.As<v8::Object>(), "windows", windows);
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
  // associate QObject with JS object (This association is used when emitting a signal from
  // singleton)
  ObjectStore::wrapAndInsert(sourceObj, obj, isolate);

  // create prototype object
  Local<Object> proto = Object::New(isolate);
  Util::processWithPublicMethods(metaObj, [&](const QMetaMethod& method) {
    NODE_SET_METHOD(proto, method.name().constData(), V8Util::invokeQObjectMethod);
  });
  JSHandler::inheritsQtEventEmitter(isolate, proto);

  // sets __proto__ (this doesn't create prototype property)
  obj->SetPrototype(proto);

  Maybe<bool> result =
      exports->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, name), obj);
  if (result.IsNothing()) {
    qWarning() << "setting exports failed";
  }
}

// register builtin silkeditbridge module
NODE_MODULE_CONTEXT_AWARE_BUILTIN(silkeditbridge, bridge::Handler::init)
