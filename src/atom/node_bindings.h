// Copyright (c) 2013 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

// Modified based on node_bindings.h in electron

#ifndef ATOM_COMMON_NODE_BINDINGS_H_
#define ATOM_COMMON_NODE_BINDINGS_H_

#include <v8.h>
#include <uv.h>
#include <QDebug>

#include "node_includes.h"
#include "core/macros.h"
#include "JSHandler.h"

namespace node {
class Environment;
}

namespace atom {

class NodeBindings {
  DISABLE_COPY(NodeBindings)
 public:
  static NodeBindings* Create();

  virtual ~NodeBindings();

  // Prepare for message loop integration.
  void PrepareMessageLoop();

  // Do message loop integration.
  virtual void RunMessageLoop();

  // Gets/sets the environment to wrap uv loop.
  void set_uv_env(node::Environment* env) { uv_env_ = env; }
  node::Environment* uv_env() const { return uv_env_; }

  void set_platform(v8::Platform* platform) { platform_ = platform; }

  // Run the libuv loop for once.
  void UvRunOnce();

  QVariant callFunc(const QString& funcName, QVariantList args = QVariantList());

  void emitSignal(QObject *obj, const QString& signal, QVariantList args);

  template <typename T>
  T callFunc(const QString& funcName, QVariantList args, T defaultValue) {
    node::Environment* env = uv_env();
    if (!env) {
      qDebug() << "NodeBinding is not yet initialized";
      return defaultValue;
    }
    v8::Locker locker(env->isolate());
    v8::HandleScope handle_scope(env->isolate());
    v8::Context::Scope context_scope(env->context());

    return JSHandler::callFunc(env->isolate(), funcName, args, defaultValue);
  }

 protected:
  NodeBindings();

  // Called to poll events in new thread.
  virtual void PollEvents() = 0;

  // Make the main thread run libuv loop.
  void WakeupMainThread();

  // Interrupt the PollEvents.
  void WakeupEmbedThread();

  // Main thread's libuv loop.
  uv_loop_t* uv_loop_;

 private:
  // Thread to poll uv events.
  static void EmbedThreadRunner(void* arg);

  // Whether the libuv loop has ended.
  bool embed_closed_;

  // Dummy handle to make uv's loop not quit.
  uv_async_t dummy_uv_handle_;

  // Thread for polling events.
  uv_thread_t embed_thread_;

  // Semaphore to wait for main loop in the embed thread.
  uv_sem_t embed_sem_;

  // Environment that to wrap the uv loop.
  node::Environment* uv_env_;

  // v8 Platform
  v8::Platform* platform_;
};

}  // namespace atom

#endif  // ATOM_COMMON_NODE_BINDINGS_H_
