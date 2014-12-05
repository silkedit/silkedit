#pragma once

#include <QObject>

#include "ICommand.h"

class CloseTabCommand : public QObject, public ICommand {
  Q_OBJECT
 public:
  static const QString name;

  CloseTabCommand();
  ~CloseTabCommand() = default;
  DEFAULT_COPY_AND_MOVE(CloseTabCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
