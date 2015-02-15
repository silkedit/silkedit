#include <QDebug>

#include "SaveAllCommand.h"
#include "SilkApp.h"
#include "TabViewGroup.h"

const QString SaveAllCommand::name = "save_all";

SaveAllCommand::SaveAllCommand() : ICommand(SaveAllCommand::name) {}

void SaveAllCommand::doRun(const CommandArgument&, int) {
  SilkApp::activeTabViewGroup()->saveAllTabs();
}
