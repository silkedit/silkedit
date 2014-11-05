#include <QDebug>

#include "ChangeModeCommand.h"
#include "../vi.h"
#include "../stlSpecialization.h"

ChangeModeCommand::ChangeModeCommand(ViEngine *viEngine)
    : ICommand("change_mode"), m_viEngine(viEngine) {}

void ChangeModeCommand::doRun(const std::unordered_map<QString, QVariant> &args) {
  if (args.find("mode") != args.end()) {
    QVariant modeVar = args.at("mode");
    if (modeVar.canConvert<QString>()) {
      QString mode = modeVar.toString().toLower();
      if (mode == "insert") {
        m_viEngine->setMode(INSERT);
      } else if (mode == "normal") {
        m_viEngine->setMode(CMD);
      } else {
        qWarning() << "invalid mode: " << mode;
      }
    }
  }
}
