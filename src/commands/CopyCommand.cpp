#include <QDebug>

#include "CopyCommand.h"
#include "API.h"
#include "TextEditView.h"

const QString CopyCommand::name = "copy";

CopyCommand::CopyCommand() : ICommand(name) {
}

void CopyCommand::doRun(const CommandArgument&, int) {
  API::activeEditView()->copy();
}
