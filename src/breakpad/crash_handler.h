#pragma once
#include <QtCore/QString>

#ifndef CRASH_HANDLER
#define CRASH_HANDLER

namespace Breakpad {
class CrashHandlerPrivate;
class CrashHandler {
 public:
  static CrashHandler* instance();
  void Init(const QString& reportPath);

  void setReportCrashesToSystem(bool report);
  bool writeMinidump();

 private:
  CrashHandler();
  ~CrashHandler();
  Q_DISABLE_COPY(CrashHandler)
  CrashHandlerPrivate* d;
};
}

#endif  // CRASH_HANDLER
