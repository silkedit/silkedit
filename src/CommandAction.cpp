#include "CommandAction.h"
#include "CommandManager.h"
#include "KeymapManager.h"

CommandAction::CommandAction(const QString& text, const QString& cmdName, QObject* parent)
    : QAction(text, parent), m_cmdName(cmdName) {
  QKeySequence key = KeymapManager::singleton().findShortcut(cmdName);
  if (!key.isEmpty()) {
    setShortcut(key);
  }
  QObject::connect(this, &QAction::triggered, [this] { CommandManager::runCommand(m_cmdName); });
}
