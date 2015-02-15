#include <QDebug>

#include "SplitHorizontallyCommand.h"
#include "SilkApp.h"
#include "TabViewGroup.h"

SplitHorizontallyCommand::SplitHorizontallyCommand() : ICommand("split_horizontally") {}

void SplitHorizontallyCommand::doRun(const CommandArgument&, int) {
  SilkApp::activeTabViewGroup()->splitTabHorizontally();
}
