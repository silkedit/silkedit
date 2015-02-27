#include <QDebug>

#include "CopyCommand.h"
#include "SilkApp.h"
#include "TextEditView.h"

const QString CopyCommand::name = "copy";

CopyCommand::CopyCommand() : ICommand(name) {
}

void CopyCommand::doRun(const CommandArgument&, int) {
  SilkApp::activeEditView()->copy();
}
