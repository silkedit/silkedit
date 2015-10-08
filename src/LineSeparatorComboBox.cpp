#include <boost/optional.hpp>

#include "LineSeparatorComboBox.h"
#include "core/LineSeparator.h"

using core::LineSeparator;

LineSeparatorComboBox::LineSeparatorComboBox(QWidget* parent) : SComboBox(parent) {
  addItemWithPopupText(LineSeparator::Windows.shortDisplayName(),
                       LineSeparator::Windows.displayName(), LineSeparator::Windows.separatorStr());
  addItemWithPopupText(LineSeparator::Unix.shortDisplayName(), LineSeparator::Unix.displayName(),
                       LineSeparator::Unix.separatorStr());
  addItemWithPopupText(LineSeparator::ClassicMac.shortDisplayName(),
                       LineSeparator::ClassicMac.displayName(),
                       LineSeparator::ClassicMac.separatorStr());
}

void LineSeparatorComboBox::setCurrentLineSeparator(const QString& separator) {
  int idx = findData(separator);
  if (idx >= 0) {
    setCurrentIndex(idx);
  } else {
    qDebug("Line separator: %s is not registered.", qPrintable(separator));
  }
}

QString LineSeparatorComboBox::currentLineSeparator() {
  return currentData().toString();
}
