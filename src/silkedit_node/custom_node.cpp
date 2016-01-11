// mostly copied from node.cc

#include "custom_node.h"
#include "atom/node_includes.h"

#include <node_crypto.h>
#include <QtGlobal>
#include <v8.h>
#include <libplatform/libplatform.h>
#include <v8-profiler.h>
#include <uv.h>

#ifdef __APPLE__
#include "atomic-polyfill.h"  // NOLINT(build/include_order)
namespace node {
template <typename T>
using atomic = nonstd::atomic<T>;
}
#else
#include <atomic>
namespace node {
template <typename T>
using atomic = std::atomic<T>;
}
#endif

namespace silkedit_node {

using v8::Array;
using v8::ArrayBuffer;
using v8::Boolean;
using v8::Context;
using v8::EscapableHandleScope;
using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::HeapStatistics;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::Locker;
using v8::Message;
using v8::Number;
using v8::Object;
using v8::ObjectTemplate;
using v8::Promise;
using v8::PromiseRejectMessage;
using v8::PropertyCallbackInfo;
using v8::SealHandleScope;
using v8::StackFrame;
using v8::StackTrace;
using v8::String;
using v8::TryCatch;
using v8::Uint32;
using v8::Uint32Array;
using v8::V8;
using v8::Value;

static node::ArrayBufferAllocator* array_buffer_allocator;
static const char** exec_argv;

static bool trace_sync_io = false;
static bool track_heap_objects = false;
static bool use_debug_agent = false;
static bool debug_wait_connect = false;
static int debug_port = 5858;

// process-relative uptime base, initialized at start-up
static bool debugger_running;
static uv_async_t dispatch_debug_messages_async;

static node::atomic<Isolate*> node_isolate;
static v8::Platform* default_platform;

static bool IsDomainActive(const node::Environment* env) {
  if (!env->using_domains())
    return false;

  Local<Array> domain_array = env->domain_array().As<Array>();
  if (domain_array->Length() == 0)
    return false;

  Local<Value> domain_v = domain_array->Get(0);
  return !domain_v->IsNull();
}

static bool ShouldAbortOnUncaughtException(Isolate* isolate) {
  HandleScope scope(isolate);

  node::Environment* env = node::Environment::GetCurrent(isolate);
  Local<Object> process_object = env->process_object();
  Local<String> emitting_top_level_domain_error_key = env->emitting_top_level_domain_error_string();
  bool isEmittingTopLevelDomainError =
      process_object->Get(emitting_top_level_domain_error_key)->BooleanValue();

  return !IsDomainActive(env) || isEmittingTopLevelDomainError;
}

// Called from V8 Debug Agent TCP thread.
static void DispatchMessagesDebugAgentCallback(node::Environment*) {
  // TODO(indutny): move async handle to environment
  uv_async_send(&dispatch_debug_messages_async);
}

static void StartDebug(node::Environment* env, bool wait) {
  Q_ASSERT(!debugger_running);

  env->debugger_agent()->set_dispatch_handler(DispatchMessagesDebugAgentCallback);
  debugger_running = env->debugger_agent()->Start(debug_port, wait);
  if (debugger_running == false) {
    fprintf(stderr, "Starting debugger on port %d failed\n", debug_port);
    fflush(stderr);
    return;
  }
}

// Called from the main thread.
static void EnableDebug(node::Environment* env) {
  CHECK(debugger_running);

  // Send message to enable debug in workers
  HandleScope handle_scope(env->isolate());

  Local<Object> message = Object::New(env->isolate());
  message->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "cmd"),
               FIXED_ONE_BYTE_STRING(env->isolate(), "NODE_DEBUG_ENABLED"));
  Local<Value> argv[] = {FIXED_ONE_BYTE_STRING(env->isolate(), "internalMessage"), message};
  node::MakeCallback(env, env->process_object(), "emit", ARRAY_SIZE(argv), argv);

  // Enabled debugger, possibly making it wait on a semaphore
  env->debugger_agent()->Enable();
}

static void RegisterSignalHandler(int signal,
                                  void (*handler)(int signal),
                                  bool reset_handler = false) {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = handler;
#ifndef __FreeBSD__
  // FreeBSD has a nasty bug with SA_RESETHAND reseting the SA_SIGINFO, that is
  // in turn set for a libthr wrapper. This leads to a crash.
  // Work around the issue by manually setting SIG_DFL in the signal handler
  sa.sa_flags = reset_handler ? SA_RESETHAND : 0;
#endif
  sigfillset(&sa.sa_mask);
  CHECK_EQ(sigaction(signal, &sa, nullptr), 0);
}

static void SignalExit(int signo) {
  uv_tty_reset_mode();
#ifdef __FreeBSD__
  // FreeBSD has a nasty bug, see RegisterSignalHandler for details
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_DFL;
  CHECK_EQ(sigaction(signo, &sa, nullptr), 0);
#endif
  raise(signo);
}

inline void PlatformInit() {
#ifdef __POSIX__
  sigset_t sigmask;
  sigemptyset(&sigmask);
  sigaddset(&sigmask, SIGUSR1);
  const int err = pthread_sigmask(SIG_SETMASK, &sigmask, nullptr);

  // Make sure file descriptors 0-2 are valid before we start logging anything.
  for (int fd = STDIN_FILENO; fd <= STDERR_FILENO; fd += 1) {
    struct stat ignored;
    if (fstat(fd, &ignored) == 0)
      continue;
    // Anything but EBADF means something is seriously wrong.  We don't
    // have to special-case EINTR, fstat() is not interruptible.
    if (errno != EBADF)
      ABORT();
    if (fd != open("/dev/null", O_RDWR))
      ABORT();
  }

  CHECK_EQ(err, 0);

  // Restore signal dispositions, the parent process may have changed them.
  struct sigaction act;
  memset(&act, 0, sizeof(act));

  // The hard-coded upper limit is because NSIG is not very reliable; on Linux,
  // it evaluates to 32, 34 or 64, depending on whether RT signals are enabled.
  // Counting up to SIGRTMIN doesn't work for the same reason.
  for (unsigned nr = 1; nr < 32; nr += 1) {
    if (nr == SIGKILL || nr == SIGSTOP)
      continue;
    act.sa_handler = (nr == SIGPIPE) ? SIG_IGN : SIG_DFL;
    CHECK_EQ(0, sigaction(nr, &act, nullptr));
  }

  RegisterSignalHandler(SIGINT, SignalExit, true);
  RegisterSignalHandler(SIGTERM, SignalExit, true);

  // Raise the open file descriptor limit.
  struct rlimit lim;
  if (getrlimit(RLIMIT_NOFILE, &lim) == 0 && lim.rlim_cur != lim.rlim_max) {
    // Do a binary search for the limit.
    rlim_t min = lim.rlim_cur;
    rlim_t max = 1 << 20;
    // But if there's a defined upper bound, don't search, just set it.
    if (lim.rlim_max != RLIM_INFINITY) {
      min = lim.rlim_max;
      max = lim.rlim_max;
    }
    do {
      lim.rlim_cur = min + (max - min) / 2;
      if (setrlimit(RLIMIT_NOFILE, &lim)) {
        max = lim.rlim_cur;
      } else {
        min = lim.rlim_cur;
      }
    } while (min + 1 < max);
  }
#endif  // __POSIX__
}

static void StartNodeInstance(void* arg, atom::NodeBindings* nodeBindings) {
  node::NodeInstanceData* instance_data = static_cast<node::NodeInstanceData*>(arg);
  Isolate::CreateParams params;
  array_buffer_allocator = new node::ArrayBufferAllocator();
  params.array_buffer_allocator = array_buffer_allocator;
  Isolate* isolate = Isolate::New(params);
  // Make isolate default of current thread. (Without this, Isolate::Current() returns NULL)
  isolate->Enter();
  if (track_heap_objects) {
    isolate->GetHeapProfiler()->StartTrackingHeapObjects(true);
  }

  // Fetch a reference to the main isolate, so we have a reference to it
  // even when we need it to access it from another (debugger) thread.
  if (instance_data->is_main())
    CHECK_EQ(nullptr, node_isolate.exchange(isolate));

  {
    Locker locker(isolate);
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);

    node::Environment* env = node::CreateEnvironment(
        isolate, instance_data->event_loop(), context, instance_data->argc(), instance_data->argv(),
        instance_data->exec_argc(), instance_data->exec_argv());
    array_buffer_allocator->set_env(env);
    Context::Scope context_scope(context);

    isolate->SetAbortOnUncaughtExceptionCallback(ShouldAbortOnUncaughtException);

    // Start debug agent when argv has --debug
    if (instance_data->use_debug_agent())
      StartDebug(env, debug_wait_connect);

    // Make uv loop being wrapped by window context.
    if (nodeBindings->uv_env() == nullptr)
      nodeBindings->set_uv_env(env);

    // Give the node loop a run to make sure everything is ready.
    nodeBindings->RunMessageLoop();

    node::LoadEnvironment(env);

    env->set_trace_sync_io(trace_sync_io);

    // Enable debugger
    if (instance_data->use_debug_agent())
      EnableDebug(env);
  }
}

// copied from Start function in node.cc
void Start(int argc, char** argv, atom::NodeBindings* nodeBindings) {
  PlatformInit();

  CHECK_GT(argc, 0);

  // Hack around with the argv pointer. Used for process.title = "blah".
  argv = uv_setup_args(argc, argv);

  // This needs to run *before* V8::Initialize().  The const_cast is not
  // optional, in case you're wondering.
  int exec_argc;
  node::g_upstream_node_mode = true;
  node::Init(&argc, const_cast<const char**>(argv), &exec_argc, &exec_argv);
  nodeBindings->PrepareMessageLoop();

  #if HAVE_OPENSSL
  // V8 on Windows doesn't have a good source of entropy. Seed it from
  // OpenSSL's pool.
  v8::SetEntropySource(node::crypto::EntropySource);
  #endif

  const int thread_pool_size = 4;
  default_platform = v8::platform::CreateDefaultPlatform(thread_pool_size);
  nodeBindings->set_platform(default_platform);
  V8::InitializePlatform(default_platform);
  V8::Initialize();

  node::NodeInstanceData instance_data(node::NodeInstanceType::MAIN, uv_default_loop(), argc,
                                       const_cast<const char**>(argv), exec_argc, exec_argv,
                                       use_debug_agent);
  StartNodeInstance(&instance_data, nodeBindings);
}

void Cleanup(node::Environment* env) {
  env->set_trace_sync_io(false);
  Isolate* isolate = env->isolate();

  {
    Locker locker(isolate);
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);

    node::EmitExit(env);
    node::RunAtExit(env);

    array_buffer_allocator->set_env(nullptr);
    env->Dispose();
  }

  // Synchronize with signal handler, see TryStartDebugger.
  while (isolate != node_isolate.exchange(nullptr))
    ;  // NOLINT

  // Note don't delete isolate because s_objects of ObjectStore has unordered_map<QObject*,
  // v8::UniquePersistent<v8::Object>>
  // If we delete isolate here, crash happens at the destructor of s_objects
  //  CHECK_NE(isolate, nullptr);
  //  isolate->Exit();
  //  isolate->Dispose();
  //  isolate = nullptr;
  delete array_buffer_allocator;

  V8::Dispose();

  delete default_platform;
  default_platform = nullptr;

  delete[] exec_argv;
  exec_argv = nullptr;
}

}  // namespace silkedit_node
