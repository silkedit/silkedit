#pragma once

#include "ICommand.h"

class SelectAllCommand : public ICommand {
 public:
  static const QString name;

  SelectAllCommand();
  ~SelectAllCommand() = default;
  DEFAULT_COPY_AND_MOVE(SelectAllCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
