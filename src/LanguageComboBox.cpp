#include "LanguageComboBox.h"
#include "TmLanguage.h"

LanguageComboBox::LanguageComboBox(QWidget* parent) : QComboBox(parent) {
  foreach (Language* lang, LanguageProvider::languages()) {
    addItem(lang->name());
  }
}
