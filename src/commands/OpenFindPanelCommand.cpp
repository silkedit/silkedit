#include <QDebug>

#include "OpenFindPanelCommand.h"
#include "SilkApp.h"
#include "MainWindow.h"

void OpenFindPanelCommand::doRun(const CommandArgument&, int) {
  if (MainWindow* window = SilkApp::activeWindow()) {
    window->openFindAndReplacePanel();
  }
}
