#pragma once

#include <memory>

#include "TextEdit.h"
#include "core/Region.h"

namespace core {
class Regexp;
class Theme;
class Document;
}

class TextEdit;
class LineNumberArea;

class TextEditPrivate {
  Q_DECLARE_PUBLIC(TextEdit)
 public:
  explicit TextEditPrivate(TextEdit* textEdit);

  TextEdit* q_ptr;
  LineNumberArea* m_lineNumberArea;
  std::shared_ptr<core::Document> m_document;
  QVector<core::Region> m_searchMatchedRegions;

  void clearHighlightingCurrentLine();
  QString prevLineText(int prevCount = 1, core::Regexp* ignorePattern = nullptr);
  void indentOneLevel(QTextCursor& currentVisibleCursor);
  void outdentOneLevel(QTextCursor& currentVisibleCursor);
  void outdentCurrentLineIfNecessary();
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect&, int);
  void setTheme(core::Theme* theme);
  void clearDirtyMarker();
  void toggleHighlightingCurrentLine(bool hasSelection);
  void emitLanguageChanged(const QString& scope);
  void emitEncodingChanged(const core::Encoding& enc);
  void emitLineSeparatorChanged(const QString& lineSeparator);
  void emitBOMChanged(const core::BOM& bom);
  void setWordWrap(bool wordWrap);
  void setupConnections(std::shared_ptr<core::Document> document);
  boost::optional<core::Region> find(const QString& text,
                          int from,
                          int begin,
                          int end,
                                     core::Document::FindFlags flags);
  int tabWidth();
};
