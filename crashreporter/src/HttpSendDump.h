#pragma once
#include <qstring.h>

namespace CrashReport {

class HttpSendDump {
 public:
  HttpSendDump(const QString& f, const QString& c, const QString& v)
    : fileName(f), comment(c), version(v){
  }

  // Send dump info.
  // Return value:
  //  success json string,filed other or empty.
  QString sendDump() const;

 private:
  QString fileName;
  QString comment;
  QString version;
};
}  // namespace
