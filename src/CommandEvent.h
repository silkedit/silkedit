#pragma once

#include <boost/optional.hpp>
#include <unordered_map>
#include <memory>
#include <QString>
#include <QVariant>

#include "core/stlSpecialization.h"
#include "core/macros.h"
#include "core/AndConditionExpression.h"
#include "CommandArgument.h"

class CommandEvent {
 public:
  CommandEvent(const QString& name, const QString& source);
  CommandEvent(const QString& name, const CommandArgument& args, const QString& source);
  CommandEvent(const QString& name,
               boost::optional<core::AndConditionExpression> condition,
               const QString& source);
  CommandEvent(const QString& name,
               const CommandArgument& args,
               boost::optional<core::AndConditionExpression> condition,
               const QString& source);
  ~CommandEvent() = default;
  DEFAULT_COPY_AND_MOVE(CommandEvent)

  QString cmdName() const { return m_cmdName; }
  QString cmdDescription() const;
  boost::optional<core::AndConditionExpression> condition() const { return m_condition; }
  QString source() const { return m_source; }

  bool execute(int repeat = 1);
  bool hasCondition();

 private:
  QString m_cmdName;
  CommandArgument m_args;
  boost::optional<core::AndConditionExpression> m_condition;

  /**
   * @brief The source where this command event is defined.
   * Empty if defined in .silk/keymap.yml
   * Package name if defined in a package keymap.yml
   */
  QString m_source;
};
