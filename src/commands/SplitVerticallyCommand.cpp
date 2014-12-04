#include <QDebug>

#include "SplitVerticallyCommand.h"
#include "API.h"
#include "MainWindow.h"

SplitVerticallyCommand::SplitVerticallyCommand() : ICommand("split_vertically") {
}

void SplitVerticallyCommand::doRun(const CommandArgument&, int) {
  API::activeWindow()->splitTabVertically();
}
