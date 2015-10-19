#include "Splitter.h"

Splitter::Splitter(Qt::Orientation orientation, QWidget* parent) : QSplitter(orientation, parent) {
  setHandleWidth(0);
}
