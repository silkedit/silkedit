#include "PluginCommand.h"
#include "plugin_service/PluginService.h"

PluginCommand::PluginCommand(const QString& name) : ICommand(name) {}

void PluginCommand::doRun(const CommandArgument&, int) {
  PluginService::singleton().callExternalCommand(name());
}
