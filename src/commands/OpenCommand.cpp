#include <QDebug>
#include <QFileDialog>

#include "OpenCommand.h"
#include "DocumentService.h"
#include "ProjectService.h"
#include "vi.h"

const QString OpenCommand::name = "open_file";

OpenCommand::OpenCommand() : ICommand(OpenCommand::name) {
}

void OpenCommand::doRun(const CommandArgument&, int) {
  QFileDialog dialog(nullptr, tr("Open"), "");
  if (dialog.exec()) {
    foreach (const QString& entry, dialog.selectedFiles()) {
      qDebug("opening %s", qPrintable(entry));
      QFileInfo info(entry);
      if (info.isFile()) {
        DocumentService::open(entry);
      } else if (info.isDir()) {
        ProjectService::open(entry);
      } else {
        qWarning("%s is neither file nor directory.", qPrintable(entry));
      }
    }
  }
}
