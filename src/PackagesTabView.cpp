#include "PackagesTabView.h"
#include "PackagesView.h"

PackagesTabView::PackagesTabView() {
  addTab(new PackagesView(new AvailablePackagesViewModel(this), this), tr("Available"));
  addTab(new PackagesView(new InstalledPackagesViewModel(this), this), tr("Installed"));
}
