#include "CommandEvent.h"
#include "CommandService.h"

CommandEvent::CommandEvent(const QString& name)
    : CommandEvent(name, std::unordered_map<QString, QVariant>(), nullptr) {
}

CommandEvent::CommandEvent(const QString& name, const std::unordered_map<QString, QVariant>& args)
    : CommandEvent(name, args, nullptr) {
}

CommandEvent::CommandEvent(const QString& name, std::shared_ptr<IContext> context)
    : CommandEvent(name, std::unordered_map<QString, QVariant>(), context) {
}

CommandEvent::CommandEvent(const QString& name,
                           const std::unordered_map<QString, QVariant>& args,
                           std::shared_ptr<IContext> context)
    : m_cmdName(name), m_args(std::move(args)), m_context(context) {
}

void CommandEvent::execute() {
  if (!m_context || m_context->isSatisfied()) {
    CommandService::singleton().runCommand(m_cmdName, m_args);
  }
}
