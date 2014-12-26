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
  if (editView) {
    setCurrentLanguage(editView->language());
  } else {
    qDebug("editView is null");
  }
}

void StatusBar::setActiveTextEditViewLanguage() {
  qDebug("currentIndexChanged in langComboBox. %d", m_langComboBox->currentIndex());
  STabWidget* tabWidget = static_cast<MainWindow*>(window())->activeTabWidget();
  if (tabWidget) {
    if (TextEditView* editView = tabWidget->activeEditView()) {
      qDebug("active editView's lang: %s", qPrintable(editView->language()->scopeName));
      editView->setLanguage(m_langComboBox->currentData().toString());
    } else {
      qDebug("active TextEditView is null");
    }
  } else {
    qDebug("active tab widget is null");
  }
}

void StatusBar::setLanguage(const QString& scope) {
  qDebug("setLanguage inStatusBar. scope: %s", qPrintable(scope));
  Language* lang = LanguageProvider::languageFromScope(scope);
  setCurrentLanguage(lang);
}

void StatusBar::setCurrentLanguage(Language* lang) {
  if (lang) {
    int idx = m_langComboBox->findText(lang->name());
    if (idx >= 0) {
      m_langComboBox->setCurrentIndex(idx);
    } else {
      qDebug("lang: %s is not registered.", qPrintable(lang->name()));
    }
  }
}
