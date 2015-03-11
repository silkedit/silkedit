#include "CommandEvent.h"
#include "CommandManager.h"

CommandEvent::CommandEvent(const QString& name) : CommandEvent(name, CommandArgument(), nullptr) {
}

CommandEvent::CommandEvent(const QString& name, const CommandArgument& args)
    : CommandEvent(name, args, nullptr) {
}

CommandEvent::CommandEvent(const QString& name, std::unique_ptr<Context> context)
    : CommandEvent(name, CommandArgument(), std::move(context)) {
}

CommandEvent::CommandEvent(const QString& name,
                           const CommandArgument& args,
                           std::unique_ptr<Context> context)
    : m_cmdName(name), m_args(std::move(args)), m_context(std::move(context)) {
}

bool CommandEvent::execute(int repeat) {
  if (!m_context || m_context->isSatisfied()) {
    CommandManager::runCommand(m_cmdName, m_args, repeat);
    return true;
  }

  return false;
}

bool CommandEvent::hasContext() {
  return m_context != nullptr;
}
