#pragma once

#include "ICommand.h"
#include "TextEditView.h"

class EvalAsRubyCommand : public ICommand {
 public:
  explicit EvalAsRubyCommand(TextEditView* textEditView);
  ~EvalAsRubyCommand() = default;
  DEFAULT_COPY_AND_MOVE(EvalAsRubyCommand)

 private:
  TextEditView* m_textEditView;
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
