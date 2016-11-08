#include <QProcess>
#include <QDebug>
#include <QApplication>
#include <QDir>

#include "UpdateProcess.h"

namespace {
QString programPath() {
  return QDir::cleanPath(QApplication::applicationDirPath() + "/../Update.exe");
}
}

namespace core {

void UpdateProcess::start(QStringList arguments) {
  updater = create(arguments);
  updater->start(programPath(), arguments);
}

void UpdateProcess::startDetached(QStringList arguments) {
  updater = create(arguments);
  updater->startDetached(programPath(), arguments);
}

void UpdateProcess::waitForFinished() {
  if (updater) {
    updater->waitForFinished();
  } else {
    qWarning() << "no updater";
  }
}

QProcess* UpdateProcess::create(QStringList arguments) {
  auto updater = new QProcess(this);

  connect(updater, &QProcess::readyReadStandardOutput, this, [this, updater] {
    QString text = updater->readAllStandardOutput();
    stdoutLog += text;
  });
  connect(updater, &QProcess::readyReadStandardError, this,
          [this, updater] { this->stderrLog += updater->readAllStandardError(); });
  connect(updater, &QProcess::errorOccurred, this,
          [this, updater, arguments](QProcess::ProcessError) {
            updater->deleteLater();
            updater->terminate();
            emit errorOccured(
                QString("%1 failed. error msg: %2").arg(arguments.first()).arg(this->stderrLog));
          });
  connect(updater, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
          [this, updater](int, QProcess::ExitStatus) {
            QString text = updater->readAllStandardOutput();
            stdoutLog += text;
            updater->deleteLater();
            emit finished(this->stdoutLog);
          });

  return updater;
}

}  // namespace core
