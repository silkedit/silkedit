#pragma once

#include "ICommand.h"

class SplitVerticallyCommand : public ICommand {
 public:
  SplitVerticallyCommand();
  ~SplitVerticallyCommand() = default;
  DEFAULT_COPY_AND_MOVE(SplitVerticallyCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
