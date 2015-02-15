#include <QDebug>

#include "RedoCommand.h"
#include "SilkApp.h"

const QString RedoCommand::name = "redo";

RedoCommand::RedoCommand() : ICommand("redo") {}

void RedoCommand::doRun(const CommandArgument&, int repeat) {
  SilkApp::activeEditView()->doRedo(repeat);
}
