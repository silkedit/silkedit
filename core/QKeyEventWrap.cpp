#include <QMetaMethod>

#include "QKeyEventWrap.h"
#include "V8Util.h"
#include "atom/node_includes.h"

v8::Persistent<v8::Function> core::QKeyEventWrap::constructor;

void core::QKeyEventWrap::Init(v8::Local<v8::Object> exports) {
  v8::Isolate* isolate = exports->GetIsolate();

  // Prepare constructor template
  v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(isolate, New);
  tpl->SetClassName(v8::String::NewFromUtf8(isolate, "KeyEvent"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(tpl, "type", QKeyEventWrap::type);
  NODE_SET_PROTOTYPE_METHOD(tpl, "key", QKeyEventWrap::key);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "KeyEvent"), tpl->GetFunction());
}

core::QKeyEventWrap::QKeyEventWrap(QKeyEvent* event) : m_keyEvent(event) {}

void core::QKeyEventWrap::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();

  if (args.IsConstructCall()) {
    // If this constructor is called from C++, existing C++ instance is passed as hidden value
    v8::Local<v8::Value> wrappedObj =
        args.Callee()->GetHiddenValue(V8Util::hiddenQObjectKey(isolate));
    if (!wrappedObj.IsEmpty() && wrappedObj->IsObject()) {
      void* ptr = wrappedObj->ToObject(isolate->GetCurrentContext())
                      .ToLocalChecked()
                      ->GetAlignedPointerFromInternalField(0);
      QKeyEventWrap* keyEvent = new QKeyEventWrap(static_cast<QKeyEvent*>(ptr));
      keyEvent->Wrap(args.This());
      args.GetReturnValue().Set(args.This());
    } else {
      qCritical() << "no hidden value";
    }
  } else {
    // Invoked as plain function `MyObject(...)`, turn into construct call.
    if (args.Length() >= Q_METAMETHOD_INVOKE_MAX_ARGS) {
      qWarning() << "max # of args is " << Q_METAMETHOD_INVOKE_MAX_ARGS
                 << ". Exceeded arguments will be dropped";
    }

    v8::Local<v8::Value> argv[Q_METAMETHOD_INVOKE_MAX_ARGS];
    for (int i = 0; i < qMin(args.Length(), Q_METAMETHOD_INVOKE_MAX_ARGS); i++) {
      argv[i] = args[i];
    }

    v8::Local<v8::Function> ctor = v8::Local<v8::Function>::New(isolate, constructor);
    Q_ASSERT(!ctor.IsEmpty());
    //      v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(isolate, ctor);
    args.GetReturnValue().Set(ctor->NewInstance(isolate->GetCurrentContext(),
                                                qMin(args.Length(), Q_METAMETHOD_INVOKE_MAX_ARGS),
                                                argv)
                                  .ToLocalChecked());
  }
}

void core::QKeyEventWrap::type(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  QKeyEventWrap* keyEvent = ObjectWrap::Unwrap<QKeyEventWrap>(args.Holder());
  args.GetReturnValue().Set(
      v8::Int32::New(isolate, static_cast<int>(keyEvent->m_keyEvent->type())));
}

void core::QKeyEventWrap::key(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  QKeyEventWrap* keyEvent = ObjectWrap::Unwrap<QKeyEventWrap>(args.Holder());
  args.GetReturnValue().Set(v8::Int32::New(isolate, keyEvent->m_keyEvent->key()));
}
