#pragma once

#include <QObject>

#include "macros.h"
#include "Singleton.h"

namespace core {

class AutoUpdater;

class AutoUpdateManager : public QObject, public Singleton<AutoUpdateManager> {
  Q_OBJECT

 public:
  ~AutoUpdateManager() = default;

  void initialize();

 signals:
  void checkingForUpdate();
  void updateAvailable();
  void updateNotAvailable();
  void updateDownloaded(const QString& notes,
                        const QString& name,
                        const QDateTime& date,
                        const QString& url);
  void updateError(const QString& message);

 private:
  friend class Singleton<AutoUpdateManager>;

  AutoUpdater* m_updater;

  AutoUpdateManager();
};

}  // namespace core
