#include "CommandAction.h"
#include "CommandService.h"
#include "KeymapService.h"

CommandAction::CommandAction(const QString& text, const QString& cmdName, QObject* parent)
    : QAction(text, parent), m_cmdName(cmdName) {
  QKeySequence key = KeymapService::singleton().findShortcut(cmdName);
  if (!key.isEmpty()) {
    setShortcut(key);
  }
  QObject::connect(this, &QAction::triggered, [this] { CommandService::runCommand(m_cmdName); });
}
