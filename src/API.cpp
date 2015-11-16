#include <memory>
#include <string>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QInputDialog>

#include "API.h"
#include "Window.h"
#include "CommandManager.h"
#include "commands/PackageCommand.h"
#include "HelperProxy.h"
#include "TextEditView.h"
#include "SilkApp.h"
#include "TabView.h"
#include "TabViewGroup.h"
#include "DocumentManager.h"
#include "ProjectManager.h"
#include "KeymapManager.h"
#include "core/ConditionManager.h"
#include "PackageCondition.h"
#include "InputDialog.h"
#include "ConfigDialog.h"
#include "core/Config.h"
#include "core/modifiers.h"
#include "core/ICondition.h"
#include "util/DialogUtils.h"

using core::Config;
using core::ConditionManager;

void API::hideActiveFindReplacePanel() {
  if (Window* window = SilkApp::activeWindow()) {
    window->hideFindReplacePanel();
  }
}

API::API() {
  // Necessary to call a method with boost::optional<QString> return value
  qRegisterMetaType<boost::optional<QString>>();
}

void API::alert(const QString& msg) {
  QMessageBox msgBox;
  msgBox.setText(msg);
  msgBox.exec();
}

void API::loadKeymap(const QString& pkgName, const QString& ymlPath) {
  KeymapManager::singleton().load(ymlPath, pkgName);
}

void API::loadMenu(const QString& pkgName, const QString& ymlPath) {
  Window::loadMenu(pkgName, ymlPath);
}

void API::loadToolbar(const QString& pkgName, const QString& ymlPath) {
  Window::loadToolbar(pkgName, ymlPath);
}

void API::loadConfig(const QString& pkgName, const QString& ymlPath) {
  ConfigDialog::loadConfig(pkgName, ymlPath);
}

void API::registerCommands(QVariantList commands) {
  for (const QVariant& cmdVar : commands) {
    //    qDebug("command: %s", cmd.c_str());
    if (cmdVar.canConvert<QStringList>() && cmdVar.value<QStringList>().size() == 2) {
      const QStringList cmd = cmdVar.value<QStringList>();
      CommandManager::singleton().add(
          std::unique_ptr<ICommand>(new PackageCommand(cmd.at(0), cmd.at(1))));
    }
  }
}

void API::unregisterCommands(QList<QString> commands) {
  for (const QString& cmd : commands) {
    qDebug("unregisterCommand: %s", qPrintable(cmd));
    CommandManager::singleton().remove(cmd);
  }
}

void API::registerCondition(const QString& condition) {
  ConditionManager::add(
      condition, std::move(std::unique_ptr<core::ICondition>(new PackageCondition(condition))));
}

void API::unregisterCondition(const QString& condition) {
  ConditionManager::remove(condition);
}

TextEditView* API::activeTextEditView() {
  return SilkApp::activeTextEditView();
}

TabView* API::activeTabView() {
  return SilkApp::activeTabView();
}

TabViewGroup* API::activeTabViewGroup() {
  return SilkApp::activeTabViewGroup();
}

Window* API::activeWindow() {
  return SilkApp::activeWindow();
}

QStringList API::showFileAndFolderDialog(const QString& caption) {
  return showDialogImpl(caption, DialogUtils::MODE::FileAndDirectory);
}

QStringList API::showFilesDialog(const QString& caption) {
  return showDialogImpl(caption, DialogUtils::MODE::Files);
}

boost::optional<QString> API::showFolderDialog(const QString& caption) {
  QList<QString> paths = DialogUtils::showDialog(caption, DialogUtils::MODE::Directory);
  for (const QString& path : paths) {
    qDebug() << path;
  }
  if (paths.empty()) {
    return boost::none;
  } else {
    return paths.front();
  }
}

QStringList API::showDialogImpl(const QString& caption, DialogUtils::MODE mode) {
  QStringList paths = DialogUtils::showDialog(caption, mode);
  for (const QString& path : paths) {
    qDebug() << path;
  }
  return paths;
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
  return SilkApp::applicationVersion();
}

InputDialog* API::newInputDialog() {
  return new InputDialog();
}

void API::open(const QString& pathStr) {
  QString path = QDir::fromNativeSeparators(pathStr);
  QFileInfo info(path);
  if (info.isFile()) {
    DocumentManager::open(path);
  } else if (info.isDir()) {
    ProjectManager::open(path);
  } else {
    qWarning("%s is neither file nor directory.", qPrintable(path));
  }
}

void API::dispatchCommand(const QString& typeStr,
                          const QString& key,
                          bool autorep,
                          bool altKey,
                          bool ctrlKey,
                          bool metaKey,
                          bool shiftKey) {
  QEvent::Type type;
  if (typeStr == "keypress") {
    type = QEvent::KeyPress;
  } else if (typeStr == "keyup") {
    type = QEvent::KeyRelease;
  } else {
    qWarning("invalid key event type: %s", qPrintable(typeStr));
    return;
  }

  Qt::KeyboardModifiers modifiers = Qt::NoModifier;
  if (altKey) {
    modifiers |= Silk::AltModifier;
  }
  if (ctrlKey) {
    modifiers |= Silk::ControlModifier;
  }
  if (metaKey) {
    modifiers |= Silk::MetaModifier;
  }
  if (shiftKey) {
    modifiers |= Silk::ShiftModifier;
  }

  KeymapManager::singleton().dispatch(
      new QKeyEvent(type, QKeySequence(key)[0], modifiers, key, autorep));
}

void API::setFont(const QString& family, int size) {
  QFont font(family, size);
  Config::singleton().setFont(font);
}
