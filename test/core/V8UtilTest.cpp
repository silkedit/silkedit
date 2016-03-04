#include <libplatform/libplatform.h>
#include <vendor/node/src/node.h>
#include <QtTest/QtTest>
#include <QCompleter>

#include "V8Util.h"

using namespace v8;

namespace core {

class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
 public:
  virtual void* Allocate(size_t length) {
    void* data = AllocateUninitialized(length);
    return data == NULL ? data : memset(data, 0, length);
  }
  virtual void* AllocateUninitialized(size_t length) { return malloc(length); }
  virtual void Free(void* data, size_t) { free(data); }
};

class V8UtilTest : public QObject {
  Q_OBJECT

 private slots:

  void toV8Value() {
    // https://developers.google.com/v8/get_started

    // Initialize V8.
    V8::InitializeICU();
    // v8::CreateDefaultPlatform is not exposed in dll
    Platform* platform = node::CreateDefaultPlatform();
    V8::InitializePlatform(platform);
    V8::Initialize();

    // Create a new Isolate and make it the current one.
    ArrayBufferAllocator allocator;
    Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = &allocator;
    Isolate* isolate = Isolate::New(create_params);
    {
      Isolate::Scope isolate_scope(isolate);

      // Create a stack-allocated handle scope.
      HandleScope handle_scope(isolate);

      // Create a new context.
      Local<Context> context = Context::New(isolate);

      // Enter the context for compiling and running the hello world script.
      Context::Scope context_scope(context);

      int n = 3;
      Local<Value> value = V8Util::toV8Value(isolate, QVariant::fromValue(n));
      Q_ASSERT(value->IsInt32());
      MaybeLocal<Int32> maybeInt = value->ToInt32(context);
      Q_ASSERT(!maybeInt.IsEmpty());
      QCOMPARE(maybeInt.ToLocalChecked()->Value(), n);
    }

    // Dispose the isolate and tear down V8.
    isolate->Dispose();
    V8::Dispose();
    V8::ShutdownPlatform();
    delete platform;
  }

  void isEnum() {
    qRegisterMetaType<QCompleter::CompletionMode>("CompletionMode");
    QCompleter completer;
    int index = completer.metaObject()->indexOfProperty("completionMode");
    QVERIFY(index >= 0);
    auto var = completer.metaObject()->property(index).read(&completer);
    QVERIFY(V8Util::isEnum(var));
  }
};
}  // namespace core

QTEST_MAIN(core::V8UtilTest)
#include "V8UtilTest.moc"
