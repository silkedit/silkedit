#include "PluginCommand.h"
#include "PluginManager.h"

PluginCommand::PluginCommand(const QString& name) : ICommand(name) {
}

void PluginCommand::doRun(const CommandArgument&, int) {
  PluginManager::singleton().callExternalCommand(name());
}
