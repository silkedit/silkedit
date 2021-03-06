#include "CommandAction.h"
#include "CommandManager.h"
#include "KeymapManager.h"
#include "core/PackageManager.h"
#include "core/Package.h"
#include "core/Config.h"
#include "core/Theme.h"

using core::Config;
using core::Theme;
using core::ColorSettings;
using core::PackageManager;
using core::Package;

void CommandAction::init(const QString& id) {
  setObjectName(id);

  // WidgetShortcut makes QAction's shortcut disabled but visible in a menu
  // Shortcut is handled by KeymapManager.
  setShortcutContext(Qt::WidgetShortcut);

  updateVisibilityAndShortcut();

  connect(this, &QAction::triggered, [this](bool checked) {
    if (isCheckable()) {
      Config::singleton().setValue(objectName(), checked);
    }
    // empty command is allowed
    // e.g. checkable but empty command action
    if (!m_cmdName.isEmpty()) {
      CommandArgument args({{"menu_checked", QVariant(checked)}});
      // Copy command arguments
      args.insert(m_args.begin(), m_args.end());
      CommandManager::singleton().runCommand(m_cmdName, args);
    }
  });
  connect(&KeymapManager::singleton(), &KeymapManager::shortcutUpdated, this,
          [=](const QString& cmdName, const QKeySequence& key) {
            if (cmdName == m_cmdName && !key.isEmpty()) {
              qDebug() << "setShortcut key:" << key << "cmd:" << cmdName;
              setShortcut(key);
            }
          });
}

CommandAction::CommandAction(const QString& id,
                             const QString& text,
                             const QString& cmdName,
                             QObject* parent, const CommandArgument &args,
                             boost::optional<core::AndConditionExpression> cond,
                             const QString& pkgName)
    : PackageAction(text, pkgName, parent, cond), m_cmdName(cmdName), m_args(args) {
  init(id);
}

CommandAction::CommandAction(const QString& id,
                             const QString& cmdName,
                             const QIcon& icon,
                             QObject* parent, const CommandArgument &args,
                             boost::optional<core::AndConditionExpression> cond,
                             const QString& pkgName)
    : PackageAction(icon, id, pkgName, parent, cond), m_cmdName(cmdName), m_args(args) {
  init(id);
}

void CommandAction::updateShortcut() {
  QKeySequence key = KeymapManager::singleton().findShortcut(m_cmdName);
  if (!key.isEmpty()) {
    setShortcut(key);
  }
}

void CommandAction::updateVisibilityAndShortcut() {
  PackageAction::updateVisibilityAndShortcut();
  updateShortcut();
}

CommandAction::CommandAction(const QString& id,
                             const QString& cmdName,
                             const QMap<QString, QString>& icons,
                             QObject* parent, const CommandArgument &args,
                             boost::optional<core::AndConditionExpression> cond,
                             const QString& pkgName)
    : PackageAction(id, pkgName, parent, cond), m_icons(icons), m_cmdName(cmdName), m_args(args) {
  init(id);
  setTheme(Config::singleton().theme());
  connect(&Config::singleton(), &Config::themeChanged, this, &CommandAction::setTheme);
}

void CommandAction::setTheme(const Theme* theme) {
  if (theme && theme->isDarkTheme()) {
    setIcon(QIcon(m_icons.value("light", NULL)));
  } else {
    setIcon(QIcon(m_icons.value("dark", NULL)));
  }
}
