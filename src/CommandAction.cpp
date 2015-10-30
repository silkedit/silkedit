#include "CommandAction.h"
#include "CommandManager.h"
#include "KeymapManager.h"
#include "core/PackageManager.h"
#include "core/Package.h"

using core::PackageManager;
using core::Package;

void CommandAction::init(const QString& id, const QString& cmdName) {
  setObjectName(id);
  QKeySequence key = KeymapManager::singleton().findShortcut(cmdName);
  if (!key.isEmpty()) {
    setShortcut(key);
  }
  connect(this, &QAction::triggered, [this] { CommandManager::runCommand(m_cmdName); });
}

CommandAction::CommandAction(const QString& id,
                             const QString& text,
                             const QString& cmdName,
                             QObject* parent,
                             const QString& pkgName)
    : PackageAction(text, pkgName, parent), m_cmdName(cmdName) {
  init(id, cmdName);
}

CommandAction::CommandAction(const QString& id,
                             const QString& cmdName,
                             const QIcon& icon,
                             QObject* parent,
                             const QString& pkgName)
    : PackageAction(icon, id, pkgName, parent), m_cmdName(cmdName) {
  init(id, cmdName);
}
