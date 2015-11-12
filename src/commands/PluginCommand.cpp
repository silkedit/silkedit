#include "PluginCommand.h"
#include "PluginManager.h"

PluginCommand::PluginCommand(const QString& name, const QString& description)
    : ICommand(name, description) {}

void PluginCommand::doRun(const CommandArgument& args, int) {
  PluginManager::singleton().callExternalCommand(name(), std::move(args));
}
