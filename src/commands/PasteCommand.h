#pragma once

#include "ICommand.h"

class PasteCommand : public ICommand {
 public:
  static const QString name;

  PasteCommand();
  ~PasteCommand() = default;
  DEFAULT_COPY_AND_MOVE(PasteCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
