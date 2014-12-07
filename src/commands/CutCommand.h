#pragma once

#include "ICommand.h"

class CutCommand : public ICommand {
 public:
  static const QString name;

  CutCommand();
  ~CutCommand() = default;
  DEFAULT_COPY_AND_MOVE(CutCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
