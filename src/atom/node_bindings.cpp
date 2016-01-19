// Copyright (c) 2013 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

// Modified based on node_bindings.cc in electron

#include <libplatform/libplatform.h>
#include <string>
#include <vector>
#include <QVariantList>
#include <QCoreApplication>
#include <QThread>

#include "node_includes.h"
#include "node_bindings.h"
#include "Helper.h"

// Force all builtin modules to be referenced so they can actually run their
// DSO constructors, see http://git.io/DRIqCg.
#define REFERENCE_MODULE(name)            \
  extern "C" void _register_##name(void); \
  void (*fp_register_##name)(void) = _register_##name
// SilkEdit builtin modules.
REFERENCE_MODULE(silkeditbridge);
#undef REFERENCE_MODULE

// The "v8::Function::kLineOffsetNotFound" is exported in node.dll, but the
// linker can not find it, could be a bug of VS.
#if defined(OS_WIN) && !defined(DEBUG)
namespace v8 {
const int Function::kLineOffsetNotFound = -1;
}
#endif

namespace atom {

namespace {

// Empty callback for async handle.
void UvNoOp(uv_async_t*) {}

}  // namespace

NodeBindings::NodeBindings()
    : uv_loop_(uv_default_loop()),
      embed_closed_(false),
      uv_env_(nullptr),
      platform_(nullptr) {}

NodeBindings::~NodeBindings() {
  // Quit the embed thread.
  embed_closed_ = true;
  uv_sem_post(&embed_sem_);
  WakeupEmbedThread();

  // Wait for everything to be done.
  uv_thread_join(&embed_thread_);

  // Clear uv.
  uv_sem_destroy(&embed_sem_);
}

void NodeBindings::PrepareMessageLoop() {
  Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

  // Add dummy handle for libuv, otherwise libuv would quit when there is
  // nothing to do.
  uv_async_init(uv_loop_, &dummy_uv_handle_, UvNoOp);

  // Start worker that will interrupt main loop when having uv events.
  uv_sem_init(&embed_sem_, 0);
  uv_thread_create(&embed_thread_, EmbedThreadRunner, this);
}

void NodeBindings::RunMessageLoop() {
  Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

  // Run uv loop for once to give the uv__io_poll a chance to add all events.
  UvRunOnce();
}

void NodeBindings::UvRunOnce() {
  Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

  node::Environment* env = uv_env();

  v8::Locker locker(env->isolate());

  v8::HandleScope handle_scope(env->isolate());

  // Enter node context while dealing with uv events.
  v8::Context::Scope context_scope(env->context());

  // run javascript code
  node::PumpMessageLoop(platform_, env->isolate());

  // Deal with uv events.
  int r = uv_run(uv_loop_, UV_RUN_NOWAIT);
  if (r == 0 || uv_loop_->stop_flag != 0)
    Helper::singleton().quitApplication();  // Quit from uv.

  // Tell the worker thread to continue polling.
  uv_sem_post(&embed_sem_);
}

void NodeBindings::WakeupMainThread() {
  //  qDebug() << "WakeupMainThread";
  QMetaObject::invokeMethod(&Helper::singleton(), "uvRunOnce", Qt::QueuedConnection);
}

void NodeBindings::WakeupEmbedThread() {
  uv_async_send(&dummy_uv_handle_);
}

// static
void NodeBindings::EmbedThreadRunner(void* arg) {
  //  qDebug() << "EmbedThreadRunner start";
  NodeBindings* self = static_cast<NodeBindings*>(arg);

  while (true) {
    // Wait for the main loop to deal with events.
    uv_sem_wait(&self->embed_sem_);
    if (self->embed_closed_)
      break;

    // Wait for something to happen in uv loop.
    // Note that the PollEvents() is implemented by derived classes, so when
    // this class is being destructed the PollEvents() would not be available
    // anymore. Because of it we must make sure we only invoke PollEvents()
    // when this class is alive.
    self->PollEvents();
    if (self->embed_closed_)
      break;

    // Deal with event in main thread.
    self->WakeupMainThread();
  }
}

}  // namespace atom
