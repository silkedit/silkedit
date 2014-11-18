#include <QDebug>
#include <QFileDialog>

#include "OpenFileCommand.h"
#include "vi.h"

const QString OpenFileCommand::name = "open_file";

OpenFileCommand::OpenFileCommand(TextEditView* textEditView)
    : ICommand(OpenFileCommand::name), m_textEditView(textEditView) {
}

void OpenFileCommand::doRun(const CommandArgument&, int) {
  QString filename = QFileDialog::getOpenFileName(0, tr("Open"), "");
  if (filename.isNull())
    return;

  QFile file(filename);
  if (!file.open(QIODevice::ReadWrite))
    return;

  QTextStream in(&file);
  m_textEditView->setPlainText(in.readAll());
}
