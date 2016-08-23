#include <QDebug>
#include <QTimer>

#include "AutoUpdateManager.h"
#include "SquirrelAutoUpdater.h"

namespace {
const int INTERVAL = 60 * 60 * 1000;  // 1h
}

namespace core {

void AutoUpdateManager::initialize() {
  if (m_updater) {
    m_updater->initialize();
    m_updater->checkForUpdates();

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this] { m_updater->checkForUpdates(); });
    timer->start(INTERVAL);
  }
}

AutoUpdateManager::AutoUpdateManager() : m_updater(nullptr) {
  m_updater = new SquirrelAutoUpdater();
  connect(m_updater, &AutoUpdater::checkingForUpdate, this, &AutoUpdateManager::checkingForUpdate);
  connect(m_updater, &AutoUpdater::updateAvailable, this, &AutoUpdateManager::updateAvailable);
  connect(m_updater, &AutoUpdater::updateNotAvailable, this,
          &AutoUpdateManager::updateNotAvailable);
  connect(m_updater, &AutoUpdater::updateDownloaded, this, &AutoUpdateManager::updateDownloaded);
}

}  // namespace core
