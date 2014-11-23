#include "CommandAction.h"
#include "CommandService.h"
#include "KeymapService.h"

CommandAction::CommandAction(const QString& text, const QString& cmdName, QObject* parent)
    : QAction(text, parent), m_cmdName(cmdName) {
  if (auto key = KeymapService::singleton().findShortcut(cmdName)) {
    setShortcut(*key);
  }
  QObject::connect(
      this, &QAction::triggered, [this] { CommandService::singleton().runCommand(m_cmdName); });
}
