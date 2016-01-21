#pragma once

#include <QObject>

#include "ICommand.h"

// The command crashes the application. Use this only for debugging.
class CrashCommand : public QObject, public ICommand {
  Q_OBJECT
 public:
  static const QString name;

  CrashCommand();
  ~CrashCommand() = default;
  DEFAULT_COPY_AND_MOVE(CrashCommand)

 private:
  void doRun(const CommandArgument&, int = 1) override;
};
