#include "CrashCommand.h"

const QString CrashCommand::name = "_silkedit_crash";

CrashCommand::CrashCommand() : ICommand(name) {
}

void CrashCommand::doRun(const CommandArgument& , int ) {
  void (*a)() = nullptr;
  a();
}
