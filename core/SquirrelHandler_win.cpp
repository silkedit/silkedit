#include <QApplication>
#include <QFileInfo>
#include <QDebug>
#include <QSettings>
#include <QDir>

#include "SquirrelHandler_win.h"
#include "UpdateProcess.h"

using core::UpdateProcess;

namespace {

#ifdef BUILD_EDGE
const QString appName("SilkEdit Edge");
#else
const QString appName("SilkEdit");
#endif

// registry keys for explorer context menu
const QString regPath = QString("HKEY_CURRENT_USER\\Software\\Classes\\*\\shell\\%1").arg(appName);
const QString commandRegPath = regPath + "\\command";

void addRegistry(const QString& path, const QString& key, const QString& value) {
  QSettings settings(path, QSettings::NativeFormat);
  settings.setValue(key, value);
}

void removeRegistry(const QString& path) {
  QSettings settings(path, QSettings::NativeFormat);
  settings.remove("");
}

void installContextMenu() {
  const QString appPath = QDir::toNativeSeparators(QApplication::applicationFilePath());
  addRegistry(regPath, "Default", QObject::tr("Open with %1").arg(appName));
  addRegistry(regPath, "Icon", appPath);

  const QString command = QString("\"%1\" \"% 1\"").arg(appPath).replace("% 1", "%1");
  addRegistry(commandRegPath, "Default", command);
}

void uninstallContextMenu() {
  removeRegistry(regPath);
}

void createShortcut(const QString exeName) {
  UpdateProcess updateProcess;
  QObject::connect(&updateProcess, &UpdateProcess::errorOccured,
                   [](const QString& msg) { qCritical() << msg; });
  updateProcess.start(QStringList{"--createShortcut", exeName});
  updateProcess.waitForFinished();
}

void removeShortcut(const QString exeName) {
  UpdateProcess updateProcess;
  QObject::connect(&updateProcess, &UpdateProcess::errorOccured,
                   [](const QString& msg) { qCritical() << msg; });

  updateProcess.start(QStringList{"--removeShortcut", exeName});
  updateProcess.waitForFinished();
}
}

namespace core {

// handle arguments sent from squirrel
// https://github.com/Squirrel/Squirrel.Windows/blob/master/docs/using/custom-squirrel-events-non-cs.md
QStringList SquirrelHandler::handleArguments(QStringList arguments) {
  const QString exeName = QFileInfo(QApplication::applicationFilePath()).fileName();

  if (arguments.contains("--squirrel-install")) {
    createShortcut(exeName);
    installContextMenu();
    exit(0);
  } else if (arguments.contains("--squirrel-firstrun")) {
    qDebug() << "--squirrel-firstrun";
    arguments.removeOne("--squirrel-firstrun");
  } else if (arguments.contains("--squirrel-updated")) {
    // update shortcut and regiestry
    createShortcut(exeName);
    installContextMenu();
    exit(0);
  } else if (arguments.contains("--squirrel-obsolete")) {
    exit(0);
  } else if (arguments.contains("--squirrel-uninstall")) {
    removeShortcut(exeName);
    uninstallContextMenu();
    exit(0);
  }

  return arguments;
}

}  // namespace core
