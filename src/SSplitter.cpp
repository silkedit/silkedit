#include "SSplitter.h"

SSplitter::SSplitter(Qt::Orientation orientation, QWidget* parent) : QSplitter(orientation, parent) {
  setHandleWidth(0);
}
