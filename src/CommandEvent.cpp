#include "CommandEvent.h"
#include "CommandManager.h"

CommandEvent::CommandEvent(const QString& name) : CommandEvent(name, CommandArgument(), nullptr) {}

CommandEvent::CommandEvent(const QString& name, const CommandArgument& args)
    : CommandEvent(name, args, nullptr) {}

CommandEvent::CommandEvent(const QString& name, std::shared_ptr<ConditionExpression> condition)
    : CommandEvent(name, CommandArgument(), condition) {}

CommandEvent::CommandEvent(const QString& name,
                           const CommandArgument& args,
                           std::shared_ptr<ConditionExpression> condition)
    : m_cmdName(name), m_args(std::move(args)), m_condition(condition) {}

bool CommandEvent::execute(int repeat) {
  if (!m_condition || m_condition->isSatisfied()) {
    CommandManager::runCommand(m_cmdName, m_args, repeat);
    return true;
  }

  return false;
}

bool CommandEvent::hascondition() {
  return m_condition != nullptr;
}
