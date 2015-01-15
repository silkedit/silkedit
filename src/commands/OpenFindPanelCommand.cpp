#include <QDebug>

#include "OpenFindPanelCommand.h"
#include "API.h"
#include "MainWindow.h"

void OpenFindPanelCommand::doRun(const CommandArgument&, int) {
  if (MainWindow* window = API::activeWindow()) {
    window->openFindAndReplacePanel();
  }
}
