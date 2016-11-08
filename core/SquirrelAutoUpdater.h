#pragma once

#include <memory>

#include "AutoUpdater.h"

namespace core {

class SquirrelAutoUpdater : public AutoUpdater {
 public:
  SquirrelAutoUpdater();
  ~SquirrelAutoUpdater() = default;
  void quitAndInstall() override;
  void initialize() override;
  void checkForUpdates() override;

 private:
  class Private;
  std::unique_ptr<Private> d;
};

}  // namespace core
