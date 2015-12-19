#pragma once
#include <qstring.h>

namespace CrashReport {

class HttpSendDump {
  public:
    HttpSendDump(){}
    HttpSendDump(QString f, QString c, QString v) {
        setFile(f);
        setComment(c);
        setVersion(v);
    }

    void setFile(QString f){fileName = f;}
    void setComment(QString c){comment = c;}
    void setVersion(QString v){version = v;}

    // Send dump info.
    // Return value:
    //  success json string,filed other or empty.
    QString sendDump() const;

  private:
    QString fileName;
    QString comment;
    QString version;
};
} // namespace
