#include <QDebug>

#include "PasteCommand.h"
#include "SilkApp.h"
#include "TextEditView.h"

const QString PasteCommand::name = "paste";

PasteCommand::PasteCommand() : ICommand(name) {}

void PasteCommand::doRun(const CommandArgument&, int) { SilkApp::activeEditView()->paste(); }
