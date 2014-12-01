#include <QDebug>

#include "UndoCommand.h"
#include "API.h"

UndoCommand::UndoCommand() : ICommand("undo") {
}

void UndoCommand::doRun(const CommandArgument&, int repeat) {
  API::activeEditView()->doUndo(repeat);
}
