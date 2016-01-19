#pragma once

#include <QString>

#include "core/macros.h"
#include "ICommand.h"

class PackageCommand : public ICommand {
 public:
  PackageCommand(const QString& name, const QString& description);
  ~PackageCommand() = default;
  DEFAULT_COPY_AND_MOVE(PackageCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
