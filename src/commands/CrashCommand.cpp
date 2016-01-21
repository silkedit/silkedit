#include "CrashCommand.h"

const QString CrashCommand::name = "_silkedit_crash";

CrashCommand::CrashCommand() : ICommand(name) {
}

void CrashCommand::doRun(const CommandArgument& , int ) {
#ifndef __clang_analyzer__
  void (*a)() = nullptr;
  a();
#endif
}
