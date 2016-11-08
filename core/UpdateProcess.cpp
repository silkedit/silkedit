#include <QProcess>
#include <QThread>
#include <QDebug>

#include "UpdateProcess.h"

void core::UpdateProcess::start(QStringList arguments, bool detached) {
  updater = new QProcess(this);

  connect(updater, &QProcess::readyReadStandardOutput, this, [this] {
    QString text = updater->readAllStandardOutput();
    stdoutLog += text;
  });
  connect(updater, &QProcess::readyReadStandardError, this,
          [this] { this->stderrLog += updater->readAllStandardError(); });
  connect(updater, &QProcess::errorOccurred, this, [this, arguments](QProcess::ProcessError) {
    updater->deleteLater();
    updater->terminate();
    emit errorOccured(
        QString("%1 failed. error msg: %2").arg(arguments.first()).arg(this->stderrLog));
  });
  connect(updater, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
          [this](int, QProcess::ExitStatus) {
            QString text = updater->readAllStandardOutput();
            stdoutLog += text;
            updater->deleteLater();
            emit finished(this->stdoutLog);
          });

  //  QString program = "../Update.exe";
  QString program = "C:/Users/shinichi/AppData/Local/SilkEdit/Update.exe";

  if (detached) {
    updater->startDetached(program, arguments);
  } else {
    updater->start(program, arguments);
  }
}
