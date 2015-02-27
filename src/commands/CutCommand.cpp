#include <QDebug>

#include "CutCommand.h"
#include "SilkApp.h"
#include "TextEditView.h"

const QString CutCommand::name = "cut";

CutCommand::CutCommand() : ICommand(name) {
}

void CutCommand::doRun(const CommandArgument&, int) {
  SilkApp::activeEditView()->cut();
}
