#include <QDebug>

#include "SplitVerticallyCommand.h"
#include "SilkApp.h"
#include "TabViewGroup.h"

SplitVerticallyCommand::SplitVerticallyCommand() : ICommand("split_vertically") {}

void SplitVerticallyCommand::doRun(const CommandArgument&, int) {
  SilkApp::activeTabViewGroup()->splitTabVertically();
}
