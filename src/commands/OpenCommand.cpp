#include <QDebug>
#include <QFileDialog>
#include <QApplication>

#include "OpenCommand.h"
#include "DocumentManager.h"
#include "ProjectManager.h"
#include "vi.h"

const QString OpenCommand::name = "open_file";

OpenCommand::OpenCommand() : ICommand(OpenCommand::name) {
}

void OpenCommand::doRun(const CommandArgument&, int) {
  // On Windows, native dialog sets QApplication::activeWindow() to NULL. We need to store and
  // restore it after closing the dialog.
  // https://bugreports.qt.io/browse/QTBUG-38414
  QWidget* activeWindow = QApplication::activeWindow();
  QFileDialog dialog(nullptr, tr("Open"), "");
  if (dialog.exec()) {
    QApplication::setActiveWindow(activeWindow);
    foreach (const QString& entry, dialog.selectedFiles()) {
      qDebug("opening %s", qPrintable(entry));
      QFileInfo info(entry);
      if (info.isFile()) {
        DocumentManager::open(entry);
      } else if (info.isDir()) {
        ProjectManager::open(entry);
      } else {
        qWarning("%s is neither file nor directory.", qPrintable(entry));
      }
    }
  }
}
