#include <QProcess>
#include <QStringList>
#include <QDir>

#include "PlatformUtil.h"

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
