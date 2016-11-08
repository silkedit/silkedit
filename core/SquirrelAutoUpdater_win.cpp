#include <memory>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QDebug>
#include <QDateTime>
#include <QApplication>
#include <QFileInfo>

#include "SquirrelAutoUpdater.h"
#include "UpdateProcess.h"
#include "version.h"
#include "App.h"

namespace {

const QString releasesToApplyKey = QStringLiteral("releasesToApply");
const QString versionKey = QStringLiteral("version");
const QString releaseNotesKey = QStringLiteral("releaseNotes");
}

namespace core {

class SquirrelAutoUpdater::Private {
 public:
};

SquirrelAutoUpdater::SquirrelAutoUpdater() {
  d = std::make_unique<Private>();
}

void SquirrelAutoUpdater::quitAndInstall() {
  auto updateProcess = new UpdateProcess();
  const QString command = "--processStartAndWait";
  const QString exeName = QFileInfo(QApplication::applicationFilePath()).fileName();
  updateProcess->start(QStringList { command, exeName }, true);
  App::exit();
}

void SquirrelAutoUpdater::initialize() {}

void SquirrelAutoUpdater::checkForUpdates() {
  qDebug() << "checkForUpdates";

#if defined Q_OS_WIN64
  const QString platform = "win64";
#elif defined Q_OS_WIN32
  const QString platform = "win32";
#endif

  QString url =
      QString("https://silkedit-release-server.herokuapp.com/update/%1/%2").arg(platform).arg(VERSION);

  auto downloadProcess = new UpdateProcess();
  connect(downloadProcess, &UpdateProcess::errorOccured, this, &AutoUpdater::updateError);
  connect(downloadProcess, &UpdateProcess::finished, this, [=](const QString& data) {
    downloadProcess->deleteLater();

    qDebug() << "data:" << data;
    QString json = data.trimmed().split('\n').last();
    qDebug() << "json:" << json;
    QJsonObject rootObj = QJsonDocument::fromJson(json.toUtf8()).object();
    if (!rootObj.contains(releasesToApplyKey)) {
      emit updateError(releasesToApplyKey + " doesn't exist");
      return;
    }

    QJsonValue arrayValue = rootObj[releasesToApplyKey];
    if (!arrayValue.isArray()) {
      emit updateError(releasesToApplyKey + " is not an array");
      return;
    }

    QJsonArray array = arrayValue.toArray();
    if (array.isEmpty()) {
      emit updateNotAvailable();
      return;
    }

    emit updateAvailable();
    QJsonObject lastObj = array.last().toObject();
    QString version = lastObj[versionKey].toString();
    QString releaseNotes = lastObj[releaseNotesKey].toString();

    // update available! Try to update.
    auto updateProcess = new UpdateProcess();
    connect(updateProcess, &UpdateProcess::errorOccured, this, &AutoUpdater::updateError);
    connect(updateProcess, &UpdateProcess::finished, this, [=](const QString&) {
      updateProcess->deleteLater();

      // date is not available on Windows, so fake it.
      emit updateDownloaded(releaseNotes, version, QDateTime(), url);
    });

    updateProcess->start(QStringList { "--update", url }, false);
  });

  downloadProcess->start(QStringList { "--download", url }, false);
}

}  // namespace core
