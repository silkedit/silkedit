#include "CommandEvent.h"

CommandEvent::CommandEvent(const QString& name)
    : m_cmdName(name), m_args(std::move(std::unordered_map<QString, QVariant>())) {
}

CommandEvent::CommandEvent(const QString& name, const std::unordered_map<QString, QVariant>& args)
    : m_cmdName(name), m_args(std::move(args)) {
}
