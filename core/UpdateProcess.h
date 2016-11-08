#pragma once

#include <QObject>
#include <QProcess>

namespace core {

class UpdateProcess : public QObject {
  Q_OBJECT
 public:
  void start(QStringList arguments);
  void startDetached(QStringList arguments);
  void waitForFinished();

 signals:
  void finished(const QString& data);
  void errorOccured(const QString& message);

 private:
  QString stdoutLog;
  QString stderrLog;
  QProcess* updater;

  QProcess* create(QStringList arguments);
};

}  // namespace core
