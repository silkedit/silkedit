#include "LineSeparatorComboBox.h"
#include "LineSeparator.h"

LineSeparatorComboBox::LineSeparatorComboBox(QWidget* parent) : QComboBox(parent) {
  addItem(LineSeparator::Windows.displayName(), LineSeparator::Windows.separator());
  addItem(LineSeparator::Unix.displayName(), LineSeparator::Unix.separator());
  addItem(LineSeparator::ClassicMac.displayName(), LineSeparator::ClassicMac.separator());
}
