#include "LanguageComboBox.h"
#include "TmLanguage.h"

LanguageComboBox::LanguageComboBox(QWidget* parent) : QComboBox(parent) {
  foreach (auto& pair, LanguageProvider::langNameAndScopePairs()) {
    addItem(pair.first, pair.second);
  }
}
