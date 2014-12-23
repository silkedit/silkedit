#include <QDebug>

#include "PasteCommand.h"
#include "API.h"
#include "TextEditView.h"

const QString PasteCommand::name = "paste";

PasteCommand::PasteCommand() : ICommand(name) {}

void PasteCommand::doRun(const CommandArgument&, int) { API::activeEditView()->paste(); }
