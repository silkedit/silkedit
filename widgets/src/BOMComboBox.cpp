#include "BOMComboBox.h"
#include <boost/optional.hpp>
#include "core/BOM.h"

using core::BOM;

BOMComboBox::BOMComboBox(QWidget* parent) : ComboBox(parent) {
  const BOM& On = BOM::getBOM(BOM::BOMSwitch::On);
  const BOM& Off = BOM::getBOM(BOM::BOMSwitch::Off);
  addItemWithPopupText(On.name(), On.displayName(), On.name());
  addItemWithPopupText(Off.name(), Off.displayName(), Off.name());
  // resize is needed because currentIndexChanged isn't fired for the first time
  resize(currentIndex());
}

void BOMComboBox::setCurrentBOM(const BOM& bom) {
  int idx = findData(bom.name());
  if (idx >= 0) {
    setCurrentIndex(idx);
  } else {
    qDebug("BOM: %s is not registered.", qPrintable(bom.name()));
  }
}

BOM BOMComboBox::currentBOM() {
  return BOM::bomForName(currentData().toString()).value();
}
