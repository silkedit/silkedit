#include <boost/optional.hpp>

#include "LineSeparatorComboBox.h"
#include "LineSeparator.h"

LineSeparatorComboBox::LineSeparatorComboBox(QWidget* parent) : QComboBox(parent) {
  addItem(LineSeparator::Windows.displayName(), LineSeparator::Windows.separatorStr());
  addItem(LineSeparator::Unix.displayName(), LineSeparator::Unix.separatorStr());
  addItem(LineSeparator::ClassicMac.displayName(), LineSeparator::ClassicMac.separatorStr());
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
