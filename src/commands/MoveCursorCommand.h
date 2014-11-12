#pragma once

#include "ICommand.h"
#include "TextEditView.h"

class MoveCursorCommand : public ICommand {
 public:
  MoveCursorCommand(TextEditView* textEditView);
  ~MoveCursorCommand() = default;
  DEFAULT_COPY_AND_MOVE(MoveCursorCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;

  TextEditView* m_textEditView;
};
