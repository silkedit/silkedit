#include "crashreport.h"
#include <qdebug.h>
#include <qprocess.h>
#include <qstring.h>
#include <qtemporaryfile.h>
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

  QTemporaryFile tempFile;
  QString tempFileName = tempFile.fileTemplate();

  // command
  //[cmd.exe /c | sh -c] minidump_stackwalk <dumpFile> > <tempFile>
  QString exec;
  QStringList arg;
  QProcess process;
#if defined(Q_OS_MAC)
  // exec = "sh -c \"" + toolPath + "/minidump_stackwalk " + filePath + " > " + tempFileName + "\"";
  exec = "sh";
  arg << "-c";
#elif defined(Q_OS_WIN)
  exec = "cmd.exe";
  arg << "/c";
#endif
  arg << "\"";
  arg << toolPath + "/minidump_stackwalk";
  arg << filePath;
  arg << ">";
  arg << tempFileName;
  arg << "\"";
  qDebug() << "exec" << exec << arg;

  int ret = process.execute(exec, arg);
  if (ret < 0) {
    qDebug() << "Error execute : " << ret;
    return rep;
  }
  process.waitForFinished();
  // can not open device
  // rep = process.readAll();

  rep = loadFile(tempFileName);
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
