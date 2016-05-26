#include "PackageCommand.h"
#include "Helper.h"

PackageCommand::PackageCommand(const QString& name, const QString& description)
    : ICommand(name, description) {}

void PackageCommand::doRun(const CommandArgument& args, int) {
  Helper::singleton().runCommand(name(), std::move(args));
}
