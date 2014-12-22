#include "StatusBar.h"
#include "LanguageComboBox.h"
#include "MainWindow.h"
#include "STabWidget.h"

StatusBar::StatusBar(MainWindow* window)
    : QStatusBar(window), m_langComboBox(new LanguageComboBox) {
  addPermanentWidget(m_langComboBox);

  connect(m_langComboBox,
          SIGNAL(currentIndexChanged(int)),
          this,
          SLOT(setActiveTextEditViewLanguage()));
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

void StatusBar::setActiveTextEditViewLanguage() {
  qDebug("currentIndexChanged. %d", m_langComboBox->currentIndex());
  STabWidget* tabWidget = static_cast<MainWindow*>(window())->activeTabWidget();
  if (tabWidget) {
    if (TextEditView* editView = tabWidget->activeEditView()) {
      editView->setLanguage(m_langComboBox->currentData().toString());
    } else {
      qDebug("active TextEditView is null");
    }
  } else {
    qDebug("active tab widget is null");
  }
}
