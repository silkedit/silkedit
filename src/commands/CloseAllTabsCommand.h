#pragma once

#include <QObject>

#include "ICommand.h"

class CloseAllTabsCommand : public QObject, public ICommand {
  Q_OBJECT
 public:
  static const QString name;

  CloseAllTabsCommand();
  ~CloseAllTabsCommand() = default;
  DEFAULT_COPY_AND_MOVE(CloseAllTabsCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
