#pragma once

#include "ICommand.h"
#include "TextEditView.h"

class RedoCommand : public ICommand {
 public:
  RedoCommand();
  ~RedoCommand() = default;
  DEFAULT_COPY_AND_MOVE(RedoCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
