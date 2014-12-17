#include "StatusBar.h"
#include "LanguageComboBox.h"
#include "MainWindow.h"

StatusBar::StatusBar(MainWindow* window)
    : QStatusBar(window), m_langComboBox(new LanguageComboBox) {
  addPermanentWidget(m_langComboBox);
}

void StatusBar::onActiveTextEditViewChanged(TextEditView* editView) {
  qDebug("onActiveTextEditViewChanged");
  Q_ASSERT(editView);
  if (editView && editView->lang()) {
    Q_ASSERT(m_langComboBox);
    int idx = m_langComboBox->findText(editView->lang()->name());
    if (idx >= 0) {
      m_langComboBox->setCurrentIndex(idx);
    } else {
      qDebug("lang: %s is not registered.", qPrintable(editView->lang()->name()));
    }
  } else {
    qDebug("editView.lang() is null");
  }
}
