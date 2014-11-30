#pragma once

#include "ICommand.h"
#include "TextEditView.h"

class DeleteCommand : public ICommand {
 public:
  DeleteCommand();
  ~DeleteCommand() = default;
  DEFAULT_COPY_AND_MOVE(DeleteCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
