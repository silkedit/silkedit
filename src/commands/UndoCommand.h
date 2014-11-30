#pragma once

#include "ICommand.h"
#include "TextEditView.h"

class UndoCommand : public ICommand {
 public:
  UndoCommand();
  ~UndoCommand() = default;
  DEFAULT_COPY_AND_MOVE(UndoCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
