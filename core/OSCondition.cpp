#include <QString>
#include "OSCondition.h"

namespace core {

const QString OSCondition::name = "os";

QString OSCondition::key() {
#if defined Q_OS_WIN
  return "windows";
#elif defined Q_OS_LINUX
  return "linux";
#else
  return "mac";
#endif
}

const QString OnMacCondition::name = "onMac";

QString OnMacCondition::key() {
#if defined Q_OS_MAC
  return "true";
#else
  return "false";
#endif
}

const QString OnWindowsCondition::name = "onWindows";

QString OnWindowsCondition::key() {
#if defined Q_OS_WIN
  return "true";
#else
  return "false";
#endif
}

}  // namespace core
