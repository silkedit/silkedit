#include <QDebug>

#include "CloseTabCommand.h"
#include "API.h"
#include "STabWidget.h"

const QString CloseTabCommand::name = "close_tab";

CloseTabCommand::CloseTabCommand() : ICommand(CloseTabCommand::name) {}

void CloseTabCommand::doRun(const CommandArgument&, int) {
  API::activeTabWidget()->closeActiveTab();
}
