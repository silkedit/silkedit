#include <QDebug>

#include "CommandManager.h"
#include "Helper.h"
#include "commands/PackageCommand.h"
#include "commands/CrashCommand.h"
#include "core/V8Util.h"
#include "atom/node_includes.h"

using core::V8Util;
using core::FunctionInfo;

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

namespace {

CommandArgument toCommandArgument(QVariantMap map) {
  CommandArgument arg;

  for (const auto& key : map.keys()) {
    arg.insert(
        std::make_pair(key.toUtf8().constData(), map.value(key).toString().toUtf8().constData()));
  }

  return arg;
}
}

QString CommandManager::cmdDescription(const QString& name) {
  if (m_commands.count(name) != 0) {
    return m_commands[name]->description();
  }

  return "";
}

bool CommandManager::runCommandEventFilter(QString& cmdName, CommandArgument& cmdArgs) {
  node::Environment* env = Helper::singleton().uvEnv();
  // run command filters
  if (!env) {
    qDebug() << "env is null";
    return false;
  }

  Isolate* isolate = env->isolate();
  v8::Locker locker(env->isolate());
  v8::Context::Scope context_scope(env->context());
  v8::HandleScope handle_scope(env->isolate());

  Local<Value> argv[1];
  int argc = 1;
  Local<Object> evObj = Object::New(isolate);
  Maybe<bool> result =
      evObj->Set(isolate->GetCurrentContext(),
                 String::NewFromUtf8(isolate, "name", v8::NewStringType::kNormal).ToLocalChecked(),
                 V8Util::toV8String(isolate, cmdName));
  if (!result.FromMaybe(false)) {
    qWarning() << "failed to set name property";
    return false;
  }

  result =
      evObj->Set(isolate->GetCurrentContext(),
                 String::NewFromUtf8(isolate, "args", v8::NewStringType::kNormal).ToLocalChecked(),
                 V8Util::toV8Object(isolate, cmdArgs));
  if (!result.FromMaybe(false)) {
    qWarning() << "failed to set args property";
    return false;
  }

  argv[0] = evObj;

  Local<Function> fn = m_jsCmdEventFilter.Get(isolate);

  v8::TryCatch trycatch(isolate);
  MaybeLocal<Value> maybeHandled =
      fn->Call(isolate->GetCurrentContext(), v8::Undefined(isolate), argc, argv);
  if (trycatch.HasCaught()) {
    MaybeLocal<Value> maybeStackTrace = trycatch.StackTrace(isolate->GetCurrentContext());
    Local<Value> exception = trycatch.Exception();
    String::Utf8Value exceptionStr(exception);
    std::stringstream ss;
    ss << "error: " << *exceptionStr;
    if (!maybeStackTrace.IsEmpty()) {
      String::Utf8Value stackTraceStr(maybeStackTrace.ToLocalChecked());
      ss << " stack trace: " << *stackTraceStr;
    }
    qWarning() << ss.str().c_str();
    return false;
  } else if (maybeHandled.IsEmpty()) {
    qWarning() << "maybeResult is empty (but exception is not thrown...)";
    return false;
  }

  Local<Value> handled = maybeHandled.ToLocalChecked();
  if (!handled->IsBoolean()) {
    qWarning() << "handled is not boolean";
    return false;
  }

  MaybeLocal<Value> maybeName =
      evObj->Get(isolate->GetCurrentContext(),
                 String::NewFromUtf8(isolate, "name", v8::NewStringType::kNormal).ToLocalChecked());
  if (maybeName.IsEmpty()) {
    qWarning() << "name property is not found";
    return false;
  }

  auto nameValue = maybeName.ToLocalChecked();
  if (!nameValue->IsString()) {
    qWarning() << "name property is not string";
    return false;
  }

  cmdName = V8Util::toQString(nameValue->ToString(isolate->GetCurrentContext()).ToLocalChecked());

  MaybeLocal<Value> maybeArgs =
      evObj->Get(isolate->GetCurrentContext(),
                 String::NewFromUtf8(isolate, "args", v8::NewStringType::kNormal).ToLocalChecked());
  if (maybeArgs.IsEmpty()) {
    qWarning() << "args property is not found";
    return false;
  }

  auto argsValue = maybeArgs.ToLocalChecked();
  if (!argsValue->IsObject()) {
    qWarning() << "args property is not object";
    return false;
  }

  auto varMap = V8Util::toVariantMap(
      isolate, argsValue->ToObject(isolate->GetCurrentContext()).ToLocalChecked());
  cmdArgs = toCommandArgument(varMap);

  return handled->ToBoolean(isolate)->Value();
}

void CommandManager::runCommand(QString name, CommandArgument args, int repeat) {
  // check hidden commands first
  if (m_hiddenCommands.find(name) != m_hiddenCommands.end()) {
    m_hiddenCommands[name]->run(args, repeat);
    return;
  }

  bool result = runCommandEventFilter(name, args);
  if (result) {
    qDebug() << name << "is handled by an event filter";
    return;
  }

  if (m_commands.find(name) != m_commands.end()) {
    m_commands[name]->run(args, repeat);
  } else {
    qDebug() << "Can't find a command: " << name;
  }
}

void CommandManager::add(std::unique_ptr<ICommand> cmd) {
  m_commands[cmd->name()] = std::move(cmd);
}

void CommandManager::addHidden(std::unique_ptr<ICommand> cmd) {
  m_hiddenCommands[cmd->name()] = std::move(cmd);
}

void CommandManager::remove(const QString& name) {
  m_commands.erase(name);
  emit commandRemoved(name);
}

void CommandManager::_assignJSCommandEventFilter(FunctionInfo info) {
  Isolate* isolate = info.isolate;
  UniquePersistent<Function> perFn(isolate, info.fn);
  m_jsCmdEventFilter.Reset(info.isolate, info.fn);
}

CommandManager::CommandManager() {
  addHidden(std::move(std::unique_ptr<ICommand>(new CrashCommand())));
}

void CommandManager::add(const QString& name, const QString& description) {
  CommandManager::singleton().add(std::unique_ptr<ICommand>(new PackageCommand(name, description)));
}
