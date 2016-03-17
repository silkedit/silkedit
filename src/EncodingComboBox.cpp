#include "EncodingComboBox.h"
#include "core/Encoding.h"
#include "TextEdit.h"

using core::Encoding;

namespace {
const int AUTO_DETECT_INDEX = 0;
}

EncodingComboBox::EncodingComboBox(QWidget* parent) : ComboBox(parent) {
  addItem(tr("Auto Detect (Reload)", ""));
  insertSeparator(1);

  for (const Encoding& encoding : Encoding::availableEncodings()) {
    addItemWithPopupText(encoding.name(), encoding.displayName(), encoding.name());
  }

  // Auto Detect
  // ----------------------
  // UTF-8
  // Japanese (Shift_JIS)
  // Japanese (EUC-JP)
  // Japanese (ISO-2022-JP)
  // ----------------------  <- insert here
  // ...
  insertSeparator(6);
  // resize is needed because currentIndexChanged isn't fired for the first time
  resize(currentIndex());
}

void EncodingComboBox::setCurrentEncoding(const Encoding& encoding) {
  int idx = findData(encoding.name());
  if (idx >= 0) {
    setCurrentIndex(idx);
  } else {
    qDebug("Encoding: %s is not registered.", qPrintable(encoding.name()));
  }
}

Encoding EncodingComboBox::currentEncoding() {
  if (currentIndex() == AUTO_DETECT_INDEX) {
    return Encoding::defaultEncoding();
  } else {
    if (const boost::optional<Encoding> enc = Encoding::encodingForName(currentData().toString())) {
      return *enc;
    } else {
      qWarning("invalid encoding name: %s", qPrintable(currentData().toString()));
      return Encoding::defaultEncoding();
    }
  }
}

bool EncodingComboBox::isAutoDetectSelected() {
  return currentIndex() == AUTO_DETECT_INDEX;
}
