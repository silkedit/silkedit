#include "CheckBox.h"
#include "FindReplaceView.h"

CheckBox::CheckBox(QWidget* parent) : QCheckBox(parent) {
  if (auto frView = qobject_cast<FindReplaceView*>(parent)) {
    connect(this, &QCheckBox::stateChanged, frView, &FindReplaceView::highlightMatches);
  }
}
