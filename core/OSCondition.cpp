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

}  // namespace core
