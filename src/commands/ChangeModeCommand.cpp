#include <QDebug>

#include "ChangeModeCommand.h"
#include "vi.h"
#include "stlSpecialization.h"
#include "ViEngine.h"

ChangeModeCommand::ChangeModeCommand(ViEngine* viEngine)
    : ICommand(ChangeModeCommand::name), m_viEngine(viEngine) {
}

const QString ChangeModeCommand::name = "change_mode";

void ChangeModeCommand::doRun(const CommandArgument& args, int) {
  if (args.contains<QString>("mode")) {
    auto mode = args.value<QString>("mode");
    if (mode == "insert") {
      m_viEngine->setMode(Mode::INSERT);
    } else if (mode == "normal") {
      m_viEngine->setMode(Mode::CMD);
    } else if (mode == "commandline") {
      m_viEngine->setMode(Mode::CMDLINE);
    } else {
      qWarning() << "invalid mode: " << mode;
    }
  }
}
