#include <QDebug>

#include "SplitVerticallyCommand.h"
#include "API.h"
#include "TabViewGroup.h"

SplitVerticallyCommand::SplitVerticallyCommand() : ICommand("split_vertically") {}

void SplitVerticallyCommand::doRun(const CommandArgument&, int) {
  API::activeTabViewGroup()->splitTabVertically();
}
