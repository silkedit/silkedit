#include "CommandEvent.h"
#include "CommandManager.h"

using core::ConditionExpression;

const QString CommandEvent::USER_KEYMAP_SOURCE = "custom";

CommandEvent::CommandEvent(const QString& name, const QString& source, int priority)
    : CommandEvent(name, CommandArgument(), boost::none, source, priority) {}

CommandEvent::CommandEvent(const QString& name,
                           const CommandArgument& args,
                           const QString& source,
                           int priority)
    : CommandEvent(name, args, boost::none, source, priority) {}

CommandEvent::CommandEvent(const QString& name,
                           boost::optional<core::AndConditionExpression> condition,
                           const QString& source,
                           int priority)
    : CommandEvent(name, CommandArgument(), condition, source, priority) {}

CommandEvent::CommandEvent(const QString& name,
                           const CommandArgument& args,
                           boost::optional<core::AndConditionExpression> condition,
                           const QString& source,
                           int priority)
    : m_cmdName(name),
      m_args(args),
      m_condition(condition),
      m_source(source),
      m_priority(priority) {}

QString CommandEvent::cmdDescription() const {
  return CommandManager::singleton().cmdDescription(m_cmdName);
}

bool CommandEvent::execute(int repeat) {
  if (!m_condition || m_condition->isSatisfied()) {
    CommandManager::singleton().runCommand(m_cmdName, m_args, repeat);
    return true;
  }

  return false;
}

bool CommandEvent::hasCondition() {
  return m_condition != boost::none;
}
