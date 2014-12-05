#pragma once

#include <QObject>

#include "ICommand.h"

class SaveAllCommand : public QObject, public ICommand {
  Q_OBJECT
 public:
  static const QString name;

  SaveAllCommand();
  ~SaveAllCommand() = default;
  DEFAULT_COPY_AND_MOVE(SaveAllCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
