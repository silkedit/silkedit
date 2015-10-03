#pragma once

#include <QAction>

#include "core/macros.h"

class CommandAction : public QAction {
 public:
  CommandAction(const QString& id,
                const QString& text,
                const QString& cmdName,
                QObject* parent = nullptr);
  CommandAction(const QString& id,
                const QString& text,
                const QString& cmdName,
                const QIcon& icon,
                QObject* parent = nullptr);
  ~CommandAction() = default;
  DEFAULT_COPY_AND_MOVE(CommandAction)

 private:
  QString m_cmdName;
  void init(const QString& id, const QString& cmdName);
};
