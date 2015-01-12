#pragma once

#include <QObject>

#include "ICommand.h"

class OpenCommand : public QObject, public ICommand {
  Q_OBJECT
 public:
  static const QString name;

  OpenCommand();
  ~OpenCommand() = default;
  DEFAULT_COPY_AND_MOVE(OpenCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
