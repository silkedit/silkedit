#include <memory>

#include "SquirrelAutoUpdater.h"

namespace core {

class SquirrelAutoUpdater::Private {
 public:
};

SquirrelAutoUpdater::SquirrelAutoUpdater() {
  d = std::make_unique<Private>();
}

void SquirrelAutoUpdater::initialize() {}

void SquirrelAutoUpdater::checkForUpdates() {}

}  // namespace core
