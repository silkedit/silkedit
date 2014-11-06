#pragma once

#include <unordered_map>
#include <QString>
#include <QVariant>

#include "stlSpecialization.h"
#include "macros.h"
#include "IContext.h"

class CommandEvent {
  DISABLE_COPY(CommandEvent)
 public:
  explicit CommandEvent(const QString& name);
  CommandEvent(const QString& name, const std::unordered_map<QString, QVariant>& args);
  CommandEvent(const QString& name, std::shared_ptr<IContext> context);
  CommandEvent(const QString& name,
               const std::unordered_map<QString, QVariant>& args,
               std::shared_ptr<IContext> context);
  ~CommandEvent() = default;
  DEFAULT_MOVE(CommandEvent)

  void execute();

 private:
  QString m_cmdName;
  std::unordered_map<QString, QVariant> m_args;
  std::shared_ptr<IContext> m_context;
};
