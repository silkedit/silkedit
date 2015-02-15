#include <QDebug>

#include "SaveFileCommand.h"
#include "SilkApp.h"
#include "TextEditView.h"

const QString SaveFileCommand::name = "save";

SaveFileCommand::SaveFileCommand() : ICommand(SaveFileCommand::name) {}

void SaveFileCommand::doRun(const CommandArgument&, int) { SilkApp::activeEditView()->save(); }
