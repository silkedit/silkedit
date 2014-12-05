#include <QDebug>

#include "SaveFileCommand.h"
#include "API.h"
#include "TextEditView.h"

const QString SaveFileCommand::name = "save_file";

SaveFileCommand::SaveFileCommand() : ICommand(SaveFileCommand::name) {
}

void SaveFileCommand::doRun(const CommandArgument&, int) {
  API::activeEditView()->save();
}
