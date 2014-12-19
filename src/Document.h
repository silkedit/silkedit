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

  QString path() { return m_path; }
  void setPath(const QString& path) { m_path = path; }
  Language* language() { return m_lang.get(); }
  void setLanguage(const QString& scopeName);

 private:
  QString m_path;
  std::unique_ptr<Language> m_lang;
  std::unique_ptr<SyntaxHighlighter> m_syntaxHighlighter;

  Document(const QString& path, const QString& text);
};
