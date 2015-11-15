#include "PackageCommand.h"
#include "HelperProxy.h"

PackageCommand::PackageCommand(const QString &name, const QString &description): ICommand(name, description)
{

}

void PackageCommand::doRun(const CommandArgument& args, int) {
  HelperProxy::singleton().callExternalCommand(name(), std::move(args));
}
