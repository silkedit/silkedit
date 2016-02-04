#include "LanguageComboBox.h"
#include "core/LanguageParser.h"

using core::LanguageProvider;

LanguageComboBox::LanguageComboBox(QWidget* parent) : ComboBox(parent) {
  foreach (auto& pair, LanguageProvider::scopeAndLangNamePairs()) {
    auto lang = LanguageProvider::languageFromScope(pair.first);
    if (lang && !lang->hideFromUser) {
      addItem(pair.second, pair.first);
    }
  }
  // resize is needed because currentIndexChanged isn't fired for the first time
  resize(currentIndex());
}
