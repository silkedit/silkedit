#include "NewFileCommand.h"
#include "API.h"
#include "TabView.h"

const QString NewFileCommand::name = "new_file";

NewFileCommand::NewFileCommand() : ICommand(NewFileCommand::name) {}

void NewFileCommand::doRun(const CommandArgument&, int) {
  API::activeTabView()->addNew();
}
