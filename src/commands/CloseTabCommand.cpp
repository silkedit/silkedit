#include <QDebug>

#include "CloseTabCommand.h"
#include "SilkApp.h"
#include "TabView.h"
#include "MainWindow.h"

const QString CloseTabCommand::name = "close_tab";

CloseTabCommand::CloseTabCommand() : ICommand(CloseTabCommand::name) {}

void CloseTabCommand::doRun(const CommandArgument&, int) {
  if (TabView* tabView = SilkApp::activeTabView()) {
    tabView->closeActiveTab();
  } else if (MainWindow* window = SilkApp::activeWindow()) {
    window->close();
  }
}
