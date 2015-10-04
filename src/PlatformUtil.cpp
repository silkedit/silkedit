#include <QProcess>
#include <QStringList>
#include <QDir>

#include "PlatformUtil.h"
#include "MenuBar.h"
#include "Window.h"
#include "util/YamlUtils.h"

#ifdef Q_OS_MAC
extern void qt_set_sequence_auto_mnemonic(bool b);
#endif

void PlatformUtil::showInFinder(const QString& filePath) {
#ifdef Q_OS_MAC
  QStringList args;
  args << "-e";
  args << "tell application \"Finder\"";
  args << "-e";
  args << "activate";
  args << "-e";
  args << "select POSIX file \"" + filePath + "\"";
  args << "-e";
  args << "end tell";
  QProcess::startDetached("osascript", args);
#endif

#ifdef Q_OS_WIN
  QStringList args;
  args << "/select," << QDir::toNativeSeparators(filePath);
  QProcess::startDetached("explorer", args);
#endif
}

QString PlatformUtil::showInFinderText() {
#ifdef Q_OS_MAC
  return QObject::tr("Show in Finder");
#endif

#ifdef Q_OS_WIN
  return QObject::tr("Show in Explorer");
#endif

#ifdef Q_OS_LINUX
  return QObject::tr("Show in Finder");
#endif
}

void PlatformUtil::enableMnemonicOnMac() {
#ifdef Q_OS_MAC
  qt_set_sequence_auto_mnemonic(true);
#endif
}

#ifdef Q_OS_MAC
void PlatformUtil::parseMenusNode(const std::string& pkgName, const YAML::Node& menusNode) {
  // There's only 1 global menu bar on Mac.
  YamlUtils::parseMenusNode(pkgName, MenuBar::globalMenuBar(), menusNode);
}

#elif defined Q_OS_WIN
void PlatformUtil::parseMenusNode(const std::string& pkgName,
                                  const YAML::Node& menusNode,
                                  QList<Window*> windows) {
  // Menu bar belongs to each window.
  foreach (Window* win, windows) { YamlUtils::parseMenusNode(pkgName, win->menuBar(), menusNode); }
}
#endif
