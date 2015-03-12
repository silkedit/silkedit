#include "PluginCommand.h"
#include "PluginManager.h"

PluginCommand::PluginCommand(const QString& name) : ICommand(name) {
}

void PluginCommand::doRun(const CommandArgument& args, int) {
  PluginManager::singleton().callExternalCommand(name(), std::move(args.args()));
}
