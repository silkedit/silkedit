#include <QDebug>

#include "CloseAllTabsCommand.h"
#include "API.h"
#include "TabView.h"

const QString CloseAllTabsCommand::name = "close_all_tabs";

CloseAllTabsCommand::CloseAllTabsCommand() : ICommand(CloseAllTabsCommand::name) {}

void CloseAllTabsCommand::doRun(const CommandArgument&, int) {
  API::activeTabView()->closeAllTabs();
}
