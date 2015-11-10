#include "CommandEvent.h"
#include "CommandManager.h"

CommandEvent::CommandEvent(const QString& name, const QString& source)
    : CommandEvent(name, CommandArgument(), nullptr, source) {}

CommandEvent::CommandEvent(const QString& name, const CommandArgument& args, const QString& source)
    : CommandEvent(name, args, nullptr, source) {}

CommandEvent::CommandEvent(const QString& name,
                           std::shared_ptr<ConditionExpression> condition,
                           const QString& source)
    : CommandEvent(name, CommandArgument(), condition, source) {}

CommandEvent::CommandEvent(const QString& name,
                           const CommandArgument& args,
                           std::shared_ptr<ConditionExpression> condition,
                           const QString& source)
    : m_cmdName(name), m_args(std::move(args)), m_condition(condition), m_source(source) {}

QString CommandEvent::cmdDescription() const {
  return CommandManager::cmdDescription(m_cmdName);
}

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
