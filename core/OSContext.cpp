#include <QString>
#include "OSContext.h"

namespace core {

const QString OSContext::name = "os";

QString OSContext::key() {
#if defined Q_OS_WIN
  return "windows";
#elif defined Q_OS_LINUX
  return "linux";
#else
  return "mac";
#endif
}

}  // namespace core
