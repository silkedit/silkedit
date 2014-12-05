#pragma once

#include <QObject>

#include "ICommand.h"

class CloseFileCommand : public QObject, public ICommand {
  Q_OBJECT
 public:
  static const QString name;

  CloseFileCommand();
  ~CloseFileCommand() = default;
  DEFAULT_COPY_AND_MOVE(CloseFileCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
