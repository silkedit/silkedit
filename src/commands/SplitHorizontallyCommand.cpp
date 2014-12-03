#include <QDebug>

#include "SplitHorizontallyCommand.h"
#include "API.h"
#include "MainWindow.h"

SplitHorizontallyCommand::SplitHorizontallyCommand() : ICommand("split_horizontally") {
}

void SplitHorizontallyCommand::doRun(const CommandArgument&, int) {
  API::activeWindow()->splitTabHorizontally();
}
