#include "LanguageComboBox.h"
#include "core/LanguageParser.h"

using core::LanguageProvider;

LanguageComboBox::LanguageComboBox(QWidget* parent) : ComboBox(parent) {
  auto pairs = LanguageProvider::scopeAndLangNamePairs();
  std::sort(pairs.begin(), pairs.end(), [](QPair<QString, QString> x, QPair<QString, QString> y) {
    return x.second < y.second;
  });
  foreach (const auto& pair, pairs) {
    auto lang = LanguageProvider::languageFromScope(pair.first);
    if (lang && !lang->hideFromUser) {
      addItem(pair.second, pair.first);
    }
  }
  // resize is needed because currentIndexChanged isn't fired for the first time
  resize(currentIndex());
}
