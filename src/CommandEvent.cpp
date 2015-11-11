#include "CommandEvent.h"
#include "CommandManager.h"

using core::ConditionExpression;

CommandEvent::CommandEvent(const QString& name, const QString& source)
    : CommandEvent(name, CommandArgument(), boost::none, source) {}

CommandEvent::CommandEvent(const QString& name, const CommandArgument& args, const QString& source)
    : CommandEvent(name, args, boost::none, source) {}

CommandEvent::CommandEvent(const QString& name,
                           boost::optional<ConditionExpression> condition,
                           const QString& source)
    : CommandEvent(name, CommandArgument(), condition, source) {}

CommandEvent::CommandEvent(const QString& name,
                           const CommandArgument& args,
                           boost::optional<ConditionExpression> condition,
                           const QString& source)
    : m_cmdName(name), m_args(args), m_condition(condition), m_source(source) {}

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

bool CommandEvent::hasCondition() {
  return m_condition != boost::none;
}
