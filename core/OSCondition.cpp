#include <QString>

#include "OSCondition.h"

namespace core {

const QString OSCondition::name = "os";

QVariant OSCondition::keyValue() {
#if defined Q_OS_WIN
  return QVariant::fromValue(QStringLiteral("windows"));
#elif defined Q_OS_LINUX
  return QVariant::fromValue(QStringLiteral("linux"));
#else
  return QVariant::fromValue(QStringLiteral("mac"));
#endif
}

const QString OnMacCondition::name = "onMac";

QVariant OnMacCondition::keyValue() {
#if defined Q_OS_MAC
  return QVariant::fromValue(true);
#else
  return QVariant::fromValue(false);
#endif
}

const QString OnWindowsCondition::name = "onWindows";

QVariant OnWindowsCondition::keyValue() {
#if defined Q_OS_WIN
  return QVariant::fromValue(true);
#else
  return QVariant::fromValue(false);
#endif
}

}  // namespace core
