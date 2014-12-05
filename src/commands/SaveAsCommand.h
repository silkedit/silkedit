#pragma once

#include <QObject>

#include "ICommand.h"

class SaveAsCommand : public QObject, public ICommand {
  Q_OBJECT
 public:
  static const QString name;

  SaveAsCommand();
  ~SaveAsCommand() = default;
  DEFAULT_COPY_AND_MOVE(SaveAsCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
