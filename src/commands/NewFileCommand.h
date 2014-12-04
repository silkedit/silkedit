#pragma once

#include "ICommand.h"

class NewFileCommand : public ICommand {
 public:
  static const QString name;

  NewFileCommand();
  ~NewFileCommand() = default;
  DEFAULT_COPY_AND_MOVE(NewFileCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
