#pragma once

#include <unordered_map>
#include <QString>
#include <QVariant>

#include "stlSpecialization.h"
#include "macros.h"

class CommandEvent {
public:
  CommandEvent(const QString &name);
  CommandEvent(const QString &name, const std::unordered_map<QString, QVariant> &args);
  ~CommandEvent() = default;
  DEFAULT_COPY_AND_MOVE(CommandEvent)

  inline QString name() { return m_cmdName; }
  inline std::unordered_map<QString, QVariant> args() { return m_args; }

private:
  QString m_cmdName;
  std::unordered_map<QString, QVariant> m_args;
};
