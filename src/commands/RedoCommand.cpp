#include <QDebug>

#include "RedoCommand.h"
#include "API.h"

const QString RedoCommand::name = "redo";

RedoCommand::RedoCommand() : ICommand("redo") {
}

void RedoCommand::doRun(const CommandArgument&, int repeat) {
  API::activeEditView()->doRedo(repeat);
}
