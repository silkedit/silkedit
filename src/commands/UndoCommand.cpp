#include <QDebug>

#include "UndoCommand.h"
#include "SilkApp.h"
#include "TextEditView.h"

QString UndoCommand::name = "undo";

UndoCommand::UndoCommand() : ICommand(name) {
}

void UndoCommand::doRun(const CommandArgument&, int repeat) {
  SilkApp::activeEditView()->doUndo(repeat);
}
