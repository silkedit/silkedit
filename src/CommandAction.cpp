#include "CommandAction.h"
#include "CommandManager.h"
#include "KeymapManager.h"

CommandAction::CommandAction(const QString& id,
                             const QString& text,
                             const QString& cmdName,
                             QObject* parent)
    : QAction(text, parent), m_cmdName(cmdName) {
  setObjectName(id);
  QKeySequence key = KeymapManager::singleton().findShortcut(cmdName);
  if (!key.isEmpty()) {
    setShortcut(key);
  }
  QObject::connect(this, &QAction::triggered, [this] { CommandManager::runCommand(m_cmdName); });
}
