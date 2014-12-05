#pragma once

#include <QObject>

#include "ICommand.h"

class CloseOtherTabsCommand : public QObject, public ICommand {
  Q_OBJECT
 public:
  static const QString name;

  CloseOtherTabsCommand();
  ~CloseOtherTabsCommand() = default;
  DEFAULT_COPY_AND_MOVE(CloseOtherTabsCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
