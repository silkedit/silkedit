#include <QDebug>

#include "OpenFindPanelCommand.h"
#include "SilkApp.h"
#include "Window.h"

void OpenFindPanelCommand::doRun(const CommandArgument&, int) {
  if (Window* window = SilkApp::activeWindow()) {
    window->openFindAndReplacePanel();
  }
}
