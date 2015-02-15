#include <QDebug>
#include <QFileDialog>

#include "SaveAsCommand.h"
#include "SilkApp.h"
#include "TextEditView.h"

const QString SaveAsCommand::name = "save_as";

SaveAsCommand::SaveAsCommand() : ICommand(SaveAsCommand::name) {}

void SaveAsCommand::doRun(const CommandArgument&, int) { SilkApp::activeEditView()->saveAs(); }
