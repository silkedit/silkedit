#ifndef CRASHREPORT_H
#define CRASHREPORT_H

#include <qstring.h>
#include <string>

class CrashReport {
 public:
  CrashReport();
  CrashReport(const QString& path);

  void setPath(const QString& path);
  void setToolPath(const QString& path);

  QString report() const;

  static QString loadFile(const QString& fileName);

 private:
  QString filePath;
  QString toolPath;
};

#endif  // CRASHREPORT_H
