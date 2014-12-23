#include <QDebug>
#include <QFileDialog>

#include "OpenFileCommand.h"
#include "DocumentService.h"
#include "vi.h"

const QString OpenFileCommand::name = "open_file";

OpenFileCommand::OpenFileCommand() : ICommand(OpenFileCommand::name) {}

void OpenFileCommand::doRun(const CommandArgument&, int) {
  QString filename = QFileDialog::getOpenFileName(0, tr("Open"), "");
  if (filename.isNull())
    return;

  DocumentService::singleton().open(filename);
}
