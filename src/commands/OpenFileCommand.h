#pragma once

#include <QObject>

#include "ICommand.h"

class OpenFileCommand : public QObject, public ICommand {
  Q_OBJECT
 public:
  static const QString name;

  OpenFileCommand();
  ~OpenFileCommand() = default;
  DEFAULT_COPY_AND_MOVE(OpenFileCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
