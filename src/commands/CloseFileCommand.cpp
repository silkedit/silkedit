#include <QDebug>

#include "CloseFileCommand.h"
#include "API.h"
#include "STabWidget.h"

const QString CloseFileCommand::name = "close_file";

CloseFileCommand::CloseFileCommand() : ICommand(CloseFileCommand::name) {
}

void CloseFileCommand::doRun(const CommandArgument&, int) {
  API::activeTabWidget()->closeActiveTab();
}
