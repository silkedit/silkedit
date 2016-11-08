#pragma once

#include <QObject>
#include <QProcess>

namespace core {

class UpdateProcess : public QObject {
  Q_OBJECT
 public:
  void start(QStringList arguments, bool detached);

 signals:
  void finished(const QString& data);
  void errorOccured(const QString& message);

 private:
  QString stdoutLog;
  QString stderrLog;
  QProcess* updater;
};

} // namespace core
