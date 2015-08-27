#include "EncodingComboBox.h"
#include "Encoding.h"

EncodingComboBox::EncodingComboBox(QWidget* parent) : QComboBox(parent) {
  for (const Encoding& encoding : Encoding::availableEncodings()) {
    addItem(encoding.displayName(), encoding.name());
  }
  // Separator between default encoding + 3 Japanese encodings and others
  insertSeparator(4);
}

void EncodingComboBox::setCurrentEncoding(const Encoding& encoding) {
  int idx = findData(encoding.name());
  if (idx >= 0) {
    setCurrentIndex(idx);
  } else {
    qDebug("Encoding: %s is not registered.", qPrintable(encoding.name()));
  }
}
