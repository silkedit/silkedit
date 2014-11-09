#include <QDebug>

#include "ChangeModeCommand.h"
#include "vi.h"
#include "stlSpecialization.h"

ChangeModeCommand::ChangeModeCommand(ViEngine* viEngine)
    : ICommand("change_mode"), m_viEngine(viEngine) {
}

void ChangeModeCommand::doRun(const CommandArgument& args, int repeat) {
  if (auto mode = args.find<QString>("mode")) {
    if (*mode == "insert") {
      m_viEngine->setMode(Mode::INSERT);
    } else if (*mode == "normal") {
      m_viEngine->setMode(Mode::CMD);
    } else {
      qWarning() << "invalid mode: " << *mode;
    }
  }
}
