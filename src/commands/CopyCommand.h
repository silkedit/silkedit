#pragma once

#include "ICommand.h"

class CopyCommand : public ICommand {
 public:
  static const QString name;

  CopyCommand();
  ~CopyCommand() = default;
  DEFAULT_COPY_AND_MOVE(CopyCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
