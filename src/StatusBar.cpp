#include "StatusBar.h"
#include "LanguageComboBox.h"
#include "MainWindow.h"
#include "STabWidget.h"

StatusBar::StatusBar(MainWindow* window)
    : QStatusBar(window), m_langComboBox(new LanguageComboBox) {
  addPermanentWidget(m_langComboBox);

  QObject::connect(m_langComboBox, static_cast<void (QComboBox::*)(int)>(&LanguageComboBox::currentIndexChanged), [this, window](int index) {
    qDebug("currentIndexChanged. %d", index);
    STabWidget* tabWidget = window->activeTabWidget();
    if (tabWidget) {
      if (TextEditView* editView = tabWidget->activeEditView()) {
        editView->setLanguage(m_langComboBox->currentData().toString());
      }
    }
  });
}

void StatusBar::onActiveTextEditViewChanged(TextEditView* editView) {
  qDebug("onActiveTextEditViewChanged");
  Q_ASSERT(editView);
  if (editView && editView->language()) {
    Q_ASSERT(m_langComboBox);
    int idx = m_langComboBox->findText(editView->language()->name());
    if (idx >= 0) {
      m_langComboBox->setCurrentIndex(idx);
    } else {
      qDebug("lang: %s is not registered.", qPrintable(editView->language()->name()));
    }
  } else {
    qDebug("editView.lang() is null");
  }
}
