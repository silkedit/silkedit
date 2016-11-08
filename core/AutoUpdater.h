#pragma once

#include <memory>
#define QT_NO_SIGNALS_SLOTS_KEYWORDS
#include <QObject>
#undef QT_NO_SIGNALS_SLOTS_KEYWORDS

namespace core {

class AutoUpdater : public QObject {
  Q_OBJECT

 public:
  AutoUpdater() = default;
  virtual ~AutoUpdater() = default;

  virtual void quitAndInstall () = 0;
  virtual void initialize() = 0;
  virtual void checkForUpdates() = 0;

  // clang-format off
  Q_SIGNALS:
  void checkingForUpdate();
  void updateAvailable();
  void updateNotAvailable();
  void updateDownloaded(const QString& notes,
                      const QString& name,
                      const QDateTime& date,
                      const QString& url);
  void updateError(const QString& message);
};

}  // namespace core
