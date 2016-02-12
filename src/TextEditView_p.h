#pragma once

#include <memory>

#include "TextEditView.h"
#include "core/Region.h"

namespace core {
class Regexp;
class Theme;
class Document;
}

class TextEditView;
class LineNumberArea;

class TextEditViewPrivate {
  Q_DECLARE_PUBLIC(TextEditView)
 public:
  explicit TextEditViewPrivate(TextEditView* editView);

  TextEditView* q_ptr;
  LineNumberArea* m_lineNumberArea;
  std::shared_ptr<core::Document> m_document;
  QVector<core::Region> m_searchMatchedRegions;
  QStringListModel* m_model;
  QCompleter* m_completer;
  bool m_completedAndSelected;

  void clearHighlightingCurrentLine();
  void performCompletion(const QString& completionPrefix);
  void insertCompletion(const QString& completion);
  void insertCompletion(const QString& completion, bool singleWord);
  void populateModel(const QString& completionPrefix);
  bool handledCompletedAndSelected(QKeyEvent* event);
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
  void setTabStopWidthFromSession();
  void setupConnections(std::shared_ptr<core::Document> document);
  boost::optional<core::Region> find(const QString& text,
                          int from,
                          int begin,
                          int end,
                                     core::Document::FindFlags flags);
};
