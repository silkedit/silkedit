#include <memory>
#include <string>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>

#include "API.h"
#include "Window.h"
#include "CommandManager.h"
#include "commands/PackageCommand.h"
#include "Helper.h"
#include "TextEditView.h"
#include "App.h"
#include "TabView.h"
#include "TabViewGroup.h"
#include "DocumentManager.h"
#include "ProjectManager.h"
#include "KeymapManager.h"
#include "core/PackageCondition.h"
#include "ConfigDialog.h"
#include "core/ConditionManager.h"
#include "core/Config.h"
#include "core/modifiers.h"
#include "core/Condition.h"

using core::Config;
using core::ConditionManager;

void API::hideActiveFindReplacePanel() {
  if (Window* window = App::instance()->activeWindow()) {
    window->hideFindReplacePanel();
  }
}

QList<Window*> API::windows() {
  return Window::windows();
}

boost::optional<QString> API::getConfig(const QString& name) {
  if (Config::singleton().contains(name)) {
    return Config::singleton().value(name);
  } else {
    return boost::none;
  }
}

QString API::version() {
  return App::applicationVersion();
}

void API::setFont(const QString& family, int size) {
  QFont font(family, size);
  Config::singleton().setFont(font);
}
