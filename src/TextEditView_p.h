#pragma once

#include <QObject>
#include <memory>

#include "TextEditView.h"

class TextEditViewPrivate : public QObject {
  Q_OBJECT
 public:
  explicit TextEditViewPrivate(TextEditView* q_ptr);

  TextEditView* q;
  QWidget* m_lineNumberArea;
  std::shared_ptr<Document> m_document;
  QVector<Region> m_searchMatchedRegions;
  std::unique_ptr<QStringListModel> m_model;
  std::unique_ptr<QCompleter> m_completer;
  bool m_completedAndSelected;

  void clearHighlightingCurrentLine();
  void performCompletion(const QString& completionPrefix);
  void insertCompletion(const QString& completion);
  void insertCompletion(const QString& completion, bool singleWord);
  void populateModel(const QString& completionPrefix);
  bool handledCompletedAndSelected(QKeyEvent* event);
  QString prevLineText(int prevCount = 1, Regexp* ignorePattern = nullptr);
  void indent(QTextCursor& currentVisibleCursor);
  void outdent(QTextCursor& currentVisibleCursor);
  void outdentCurrentLineIfNecessary();
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect&, int);
  void setTheme(Theme* theme);
  void clearDirtyMarker();
  void toggleHighlightingCurrentLine(bool hasSelection);
};
