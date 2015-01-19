#pragma once

#include <QTextDocument>

#include "macros.h"
#include "LanguageParser.h"
#include "SyntaxHighlighter.h"
#include "Regexp.h"

class Document : public QTextDocument {
  Q_OBJECT
  DISABLE_COPY(Document)

 public:
  enum FindFlag {
    FindBackward = 0x00001,
    FindCaseSensitively = 0x00002,
    FindWholeWords = 0x00004,
    FindRegex = 0x00008
  };
  Q_DECLARE_FLAGS(FindFlags, FindFlag)

  ~Document();
  DEFAULT_MOVE(Document)

  static Document* create(const QString& path = "");
  static Document* createBlank();

  QString path() { return m_path; }
  void setPath(const QString& path) { m_path = path; }
  Language* language() { return m_lang.get(); }
  bool setLanguage(const QString& scopeName);
  QTextCursor find(const QString& subString, int from = 0, FindFlags options = 0) const;
  QTextCursor find(const QString& subString, const QTextCursor& from, FindFlags options = 0) const;
  QTextCursor find(const Regexp* expr, int from = 0, FindFlags options = 0) const;
  QTextCursor find(const Regexp* expr, const QTextCursor& cursor, FindFlags options = 0) const;

 private:
  QString m_path;
  std::unique_ptr<Language> m_lang;
  SyntaxHighlighter* m_syntaxHighlighter;

  Document(const QString& path, const QString& text);
  Document();

  void setupLayout();
  void setupSyntaxHighlighter(Language* lang, const QString& text = "");
};
