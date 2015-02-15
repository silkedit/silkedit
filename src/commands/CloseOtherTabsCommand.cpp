#include <QDebug>

#include "CloseOtherTabsCommand.h"
#include "SilkApp.h"
#include "TabView.h"

const QString CloseOtherTabsCommand::name = "close_other_tabs";

CloseOtherTabsCommand::CloseOtherTabsCommand() : ICommand(CloseOtherTabsCommand::name) {}

void CloseOtherTabsCommand::doRun(const CommandArgument&, int) {
  SilkApp::activeTabView()->closeOtherTabs();
}
