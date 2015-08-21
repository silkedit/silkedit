#include "EncodingComboBox.h"
#include "Encoding.h"

EncodingComboBox::EncodingComboBox(QWidget* parent) : QComboBox(parent) {
  for (const Encoding& encoding : Encoding::availableEncodings()) {
    addItem(encoding.displayName(), encoding.name());
  }
}
