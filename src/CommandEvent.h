#pragma once

#include <unordered_map>
#include <memory>
#include <QString>
#include <QVariant>

#include "core/stlSpecialization.h"
#include "core/macros.h"
#include "ConditionExpression.h"
#include "CommandArgument.h"

class CommandEvent {
  DISABLE_COPY(CommandEvent)
 public:
  CommandEvent(const QString& name, const QString& source);
  CommandEvent(const QString& name, const CommandArgument& args, const QString& source);
  CommandEvent(const QString& name,
               std::shared_ptr<ConditionExpression> condition,
               const QString& source);
  CommandEvent(const QString& name,
               const CommandArgument& args,
               std::shared_ptr<ConditionExpression> condition,
               const QString& source);
  ~CommandEvent() = default;
  DEFAULT_MOVE(CommandEvent)

  QString cmdName() const { return m_cmdName; }
  ConditionExpression* condition() const { return m_condition.get(); }
  QString source() const { return m_source; }

  bool execute(int repeat = 1);
  bool hascondition();
  void clearCondition() { m_condition.reset(); }

 private:
  QString m_cmdName;
  CommandArgument m_args;
  std::shared_ptr<ConditionExpression> m_condition;

  /**
   * @brief The source where this command event is defined.
   * Empty if defined in .silk/keymap.yml
   * Package name if defined in a package keymap.yml
   */
  QString m_source;
};
