#include <QString>
#include "OSContext.h"

const QString OSContext::name = "os";

QString OSContext::key() {
#if Q_OS_WIN
  return "win";
#elif Q_OS_LINUX
  return "linux";
#else
  return "mac";
#endif
}
