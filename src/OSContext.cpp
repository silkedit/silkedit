#include <QString>
#include "OSContext.h"

const QString OSContext::name = "os";

QString OSContext::key() {
#if defined Q_OS_WIN
  return "win";
#elif defined Q_OS_LINUX
  return "linux";
#else
  return "mac";
#endif
}
