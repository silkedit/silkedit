#include <QDebug>

#include "CloseOtherTabsCommand.h"
#include "API.h"
#include "STabWidget.h"

const QString CloseOtherTabsCommand::name = "close_other_tabs";

CloseOtherTabsCommand::CloseOtherTabsCommand() : ICommand(CloseOtherTabsCommand::name) {
}

void CloseOtherTabsCommand::doRun(const CommandArgument&, int) {
  API::activeTabWidget()->closeOtherTabs();
}
