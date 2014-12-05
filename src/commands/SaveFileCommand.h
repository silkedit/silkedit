#pragma once

#include <QObject>

#include "ICommand.h"

class SaveFileCommand : public QObject, public ICommand {
  Q_OBJECT
 public:
  static const QString name;

  SaveFileCommand();
  ~SaveFileCommand() = default;
  DEFAULT_COPY_AND_MOVE(SaveFileCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
