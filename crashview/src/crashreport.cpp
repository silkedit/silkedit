#include "crashreport.h"
#include <qdebug.h>
#include <qprocess.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>

CrashReport::CrashReport() {}

CrashReport::CrashReport(const QString& path) {
  setPath(path);
}

void CrashReport::setPath(const QString& path) {
  filePath = path;
}

void CrashReport::setToolPath(const QString& path) {
  toolPath = path;
}

QString CrashReport::report() const {
  QString rep = "";

  // command
  //minidump_stackwalk <dumpFile>
  QString exec;
  QStringList arg;
  QProcess process;

  exec = toolPath + "/minidump_stackwalk";
  arg << filePath;
  qDebug() << "exec" << exec << arg;

  //do not use 'execute'. becose can not read output.
  process.start(exec,arg);
  process.waitForFinished();
  int ret = process.exitCode();
  if (ret < 0) {
    qDebug() << "Error execute : " << ret;
    return rep;
  }
  rep = process.readAllStandardOutput();

  return rep;
}

QString CrashReport::loadFile(const QString& fileName) {
  QString cont = "";

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "Error loadFile:" << fileName;
    return cont;
  }
  qDebug() << "loadFile:" << fileName;

  QTextStream in(&file);
  cont = in.readAll();
  file.close();

  return cont;
}
