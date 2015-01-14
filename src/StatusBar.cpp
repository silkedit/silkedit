#include "StatusBar.h"
#include "LanguageComboBox.h"
#include "MainWindow.h"
#include "TabView.h"
#include "TextEditView.h"

StatusBar::StatusBar(MainWindow* window)
    : QStatusBar(window), m_langComboBox(new LanguageComboBox) {
  addPermanentWidget(m_langComboBox);

  connect(m_langComboBox,
          SIGNAL(currentIndexChanged(int)),
          this,
          SLOT(setActiveTextEditViewLanguage()));
}

void StatusBar::onActiveTextEditViewChanged(TextEditView*, TextEditView* newEditView) {
  qDebug("onActiveTextEditViewChanged");
  if (newEditView) {
    setCurrentLanguage(newEditView->language());
  } else {
    qDebug("newEditView is null");
  }
}

void StatusBar::setActiveTextEditViewLanguage() {
  qDebug("currentIndexChanged in langComboBox. %d", m_langComboBox->currentIndex());
  TabView* tabView = static_cast<MainWindow*>(window())->activeTabView();
  if (tabView) {
    if (TextEditView* editView = tabView->activeEditView()) {
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
