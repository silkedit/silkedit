#pragma once

#include <QAction>

#include "core/macros.h"
#include "core/PackageAction.h"

class CommandAction : public core::PackageAction {
  Q_OBJECT
 public:
  CommandAction(const QString& id,
                const QString& text,
                const QString& cmdName,
                QObject* parent = nullptr,
                boost::optional<core::AndConditionExpression> cond = boost::none,
                const QString& pkgName = "");
  CommandAction(const QString& id,
                const QString& cmdName,
                const QIcon& icon,
                QObject* parent = nullptr,
                boost::optional<core::AndConditionExpression> cond = boost::none,
                const QString& pkgName = "");
  ~CommandAction() = default;
  DEFAULT_COPY_AND_MOVE(CommandAction)

 private:
  QString m_cmdName;

  void init(const QString& id, const QString& cmdName);
};
