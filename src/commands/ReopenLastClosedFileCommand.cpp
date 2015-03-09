#include <QDebug>

#include "ReopenLastClosedFileCommand.h"
#include "OpenRecentItemManager.h"

const QString ReopenLastClosedFileCommand::name = "reopen_last_closed_file";

ReopenLastClosedFileCommand::ReopenLastClosedFileCommand()
    : ICommand(ReopenLastClosedFileCommand::name) {
}

void ReopenLastClosedFileCommand::doRun(const CommandArgument&, int) {
  OpenRecentItemManager::singleton().reopenLastClosedFile();
}
