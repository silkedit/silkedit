#include "LanguageComboBox.h"
#include "LanguageParser.h"

LanguageComboBox::LanguageComboBox(QWidget* parent) : QComboBox(parent) {
  foreach (auto& pair, LanguageProvider::scopeAndLangNamePairs()) {
    addItem(pair.second, pair.first);
  }
}
