#pragma once

#include "ICommand.h"
#include "ViEditView.h"

class EvalAsRubyCommand : public ICommand {
 public:
  explicit EvalAsRubyCommand(ViEditView* viEditView);
  ~EvalAsRubyCommand() = default;
  DEFAULT_COPY_AND_MOVE(EvalAsRubyCommand)

 private:
  ViEditView* m_viEditView;
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
