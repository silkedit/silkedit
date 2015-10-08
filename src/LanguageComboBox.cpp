#include "LanguageComboBox.h"
#include "core/LanguageParser.h"

using core::LanguageProvider;

LanguageComboBox::LanguageComboBox(QWidget* parent) : SComboBox(parent) {
  foreach (auto& pair, LanguageProvider::scopeAndLangNamePairs()) {
    addItem(pair.second, pair.first);
  }
}
