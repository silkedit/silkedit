#include <stextedit.h>

#include "Document.h"

Document::Document(const QString& path, const QString& text) : QTextDocument(text), m_path(path) {
  STextDocumentLayout* layout = new STextDocumentLayout(this);
  setDocumentLayout(layout);

  int dotPos = path.lastIndexOf('.');
  if (dotPos >= 0) {
    QString ext = path.mid(dotPos + 1);
    qDebug("ext: %s", qPrintable(ext));
    m_lang.reset(LanguageProvider::languageFromExtension(ext));
    if (m_lang) {
      LanguageParser* parser = LanguageParser::create(m_lang->scopeName, text);
      m_syntaxHighlighter.reset(new SyntaxHighlighter(this, parser));
      if (m_syntaxHighlighter) {
        m_syntaxHighlighter->setTheme("packages/Solarized (Light).tmTheme");
      }
    }
  } else {
    qDebug("extension not found. path: %s", qPrintable(path));
  }
}

Document* Document::create(const QString& path) {
  qDebug("Docment::create(%s)", qPrintable(path));
  QFile file(path);
  if (!file.open(QIODevice::ReadWrite))
    return nullptr;

  QTextStream in(&file);
  return new Document(path, in.readAll());
}

void Document::setLanguage(const QString& scopeName) {
  qDebug("setLanguage: %s", qPrintable(scopeName));
  m_lang.reset(LanguageProvider::languageFromScope(scopeName));
  if (m_lang && m_syntaxHighlighter) {
    LanguageParser* parser = LanguageParser::create(m_lang->scopeName, toPlainText());
    m_syntaxHighlighter->setParser(parser);
    m_syntaxHighlighter->rehighlight();
  }
}
