#pragma once

#include "ICommand.h"

class SplitHorizontallyCommand : public ICommand {
 public:
  SplitHorizontallyCommand();
  ~SplitHorizontallyCommand() = default;
  DEFAULT_COPY_AND_MOVE(SplitHorizontallyCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
