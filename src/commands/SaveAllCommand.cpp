#include <QDebug>

#include "SaveAllCommand.h"
#include "API.h"
#include "TabViewGroup.h"

const QString SaveAllCommand::name = "save_all";

SaveAllCommand::SaveAllCommand() : ICommand(SaveAllCommand::name) {}

void SaveAllCommand::doRun(const CommandArgument&, int) {
  API::activeTabViewGroup()->saveAllTabs();
}
