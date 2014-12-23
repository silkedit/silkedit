#include <QDebug>

#include "UndoCommand.h"
#include "API.h"

const QString UndoCommand::name = "undo";

UndoCommand::UndoCommand() : ICommand(name) {}

void UndoCommand::doRun(const CommandArgument&, int repeat) {
  API::activeEditView()->doUndo(repeat);
}
