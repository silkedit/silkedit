#pragma once

#include <QObject>

#include "ICommand.h"

class ReopenLastClosedFileCommand : public QObject, public ICommand {
  Q_OBJECT
 public:
  static const QString name;

  ReopenLastClosedFileCommand();
  ~ReopenLastClosedFileCommand() = default;
  DEFAULT_COPY_AND_MOVE(ReopenLastClosedFileCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
