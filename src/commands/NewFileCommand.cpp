#include "NewFileCommand.h"
#include "MainWindow.h"
#include "API.h"
#include "STabWidget.h"

const QString NewFileCommand::name = "new_file";

NewFileCommand::NewFileCommand() : ICommand(NewFileCommand::name) {}

void NewFileCommand::doRun(const CommandArgument&, int) {
  if (MainWindow* window = API::activeWindow()) {
    window->activeTabWidget()->addNew();
  } else {
    qWarning("active window is null");
  }
}
