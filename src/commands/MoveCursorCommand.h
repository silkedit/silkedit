#pragma once

#include "ICommand.h"
#include "ViEditView.h"

class MoveCursorCommand : public ICommand {
 public:
  MoveCursorCommand(ViEditView* viEditView);
  ~MoveCursorCommand() = default;
  DEFAULT_COPY_AND_MOVE(MoveCursorCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;

  ViEditView* m_viEditView;
};
