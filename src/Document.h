#pragma once

#include <QTextDocument>

#include "macros.h"
#include "LanguageParser.h"
#include "SyntaxHighlighter.h"

class Document : public QTextDocument {
  Q_OBJECT
  DISABLE_COPY(Document)

 public:
  ~Document();
  DEFAULT_MOVE(Document)

  static Document* create(const QString& path = "");
  static Document* createBlank();

  QString path() { return m_path; }
  void setPath(const QString& path) { m_path = path; }
  Language* language() { return m_lang.get(); }
  bool setLanguage(const QString& scopeName);

 private:
  QString m_path;
  std::unique_ptr<Language> m_lang;
  SyntaxHighlighter* m_syntaxHighlighter;

  Document(const QString& path, const QString& text);
  Document();

  void setupLayout();
  void setupSyntaxHighlighter(Language* lang, const QString& text = "");
};
