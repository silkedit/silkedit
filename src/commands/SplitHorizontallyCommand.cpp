#include <QDebug>

#include "SplitHorizontallyCommand.h"
#include "API.h"
#include "TabViewGroup.h"

SplitHorizontallyCommand::SplitHorizontallyCommand() : ICommand("split_horizontally") {}

void SplitHorizontallyCommand::doRun(const CommandArgument&, int) {
  API::activeTabViewGroup()->splitTabHorizontally();
}
