#include <QDebug>
#include <QFileDialog>

#include "SaveAsCommand.h"
#include "API.h"
#include "TextEditView.h"

const QString SaveAsCommand::name = "save_as";

SaveAsCommand::SaveAsCommand() : ICommand(SaveAsCommand::name) {}

void SaveAsCommand::doRun(const CommandArgument&, int) { API::activeEditView()->saveAs(); }
