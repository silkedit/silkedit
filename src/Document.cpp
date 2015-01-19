#include <QPlainTextDocumentLayout>

#include "Document.h"

Document::Document(const QString& path, const QString& text)
    : QTextDocument(text), m_path(path), m_syntaxHighlighter(nullptr) {
  setupLayout();

  int dotPos = path.lastIndexOf('.');
  if (dotPos >= 0) {
    QString ext = path.mid(dotPos + 1);
    qDebug("ext: %s", qPrintable(ext));
    setupSyntaxHighlighter(LanguageProvider::languageFromExtension(ext), toPlainText());
  } else {
    qDebug("extension not found. path: %s", qPrintable(path));
  }
}

Document::Document() {
  setupLayout();
  setupSyntaxHighlighter(LanguageProvider::defaultLanguage());
}

void Document::setupLayout() {
  QPlainTextDocumentLayout* layout = new QPlainTextDocumentLayout(this);
  setDocumentLayout(layout);
}

void Document::setupSyntaxHighlighter(Language* lang, const QString& text) {
  m_lang.reset(lang);
  if (m_lang) {
    LanguageParser* parser = LanguageParser::create(m_lang->scopeName, text);
    m_syntaxHighlighter = new SyntaxHighlighter(this, parser);
  } else {
    qDebug("lang is null");
  }
}

Document::~Document() { qDebug("~Document"); }

Document* Document::create(const QString& path) {
  qDebug("Docment::create(%s)", qPrintable(path));
  QFile file(path);
  if (!file.open(QIODevice::ReadWrite))
    return nullptr;

  QTextStream in(&file);
  return new Document(path, in.readAll());
}

Document* Document::createBlank() { return new Document(); }

bool Document::setLanguage(const QString& scopeName) {
  qDebug("setLanguage: %s", qPrintable(scopeName));
  Language* newLang = LanguageProvider::languageFromScope(scopeName);
  if (m_lang.get() == newLang || (m_lang && newLang && *m_lang == *newLang)) {
    qDebug("lang is already %s", qPrintable(scopeName));
    return false;
  }

  m_lang.reset(newLang);
  if (m_lang && m_syntaxHighlighter) {
    LanguageParser* parser = LanguageParser::create(m_lang->scopeName, toPlainText());
    m_syntaxHighlighter->setParser(parser);
    if (m_syntaxHighlighter) {
      m_syntaxHighlighter->rehighlight();
    }
  }
  return true;
}

QTextCursor Document::find(const QString& subString,
                           int from,
                           int begin,
                           int end,
                           Document::FindFlags options) const {
  if (subString.isEmpty()) {
    return QTextCursor();
  }

  bool isCaseSensitive = options & Document::FindCaseSensitively;
  bool isRegex = options & Document::FindRegex;
  bool isWholeWord = options & Document::FindWholeWords;

  QString str = isRegex ? subString : Regexp::escape(subString);

  if (isWholeWord) {
    str = "\\b" + str + "\\b";
  }

  if (isCaseSensitive) {
    str = "(?-i)" + str;
  } else {
    str = "(?i)" + str;
  }

  std::unique_ptr<Regexp> regexp(Regexp::compile(str));
  if (regexp) {
    return find(regexp.get(), from, begin, end, options);
  } else {
    return QTextCursor();
  }
}

QTextCursor Document::find(const QString& subString,
                           const QTextCursor& cursor,
                           int begin,
                           int end,
                           Document::FindFlags options) const {
  if (cursor.isNull())
    return QTextCursor();

  int pos;
  if (options & QTextDocument::FindBackward) {
    pos = cursor.selectionStart();
  } else {
    pos = cursor.selectionEnd();
  }
  return find(subString, pos, begin, end, options);
}

QTextCursor Document::find(const Regexp* expr,
                           int from,
                           int begin,
                           int end,
                           Document::FindFlags options) const {
  bool isBackward = options.testFlag(Document::FindBackward);
  qDebug("find: %s, back: %d, from: %d, begin: %d, end: %d",
         qPrintable(expr->pattern()),
         (options.testFlag(Document::FindBackward)),
         from,
         begin,
         end);
  QStringRef text = isBackward ? toPlainText().midRef(begin, from - begin)
                               : toPlainText().midRef(from, end - from);
  QVector<int>* indices = expr->findStringSubmatchIndex(text, isBackward);
  if (indices && indices->size() > 1) {
    int startPos, endPos;
    if (isBackward) {
      startPos = begin + indices->at(0);
      endPos = begin + indices->at(1);
    } else {
      startPos = from + indices->at(0);
      endPos = from + indices->at(1);
    }
    QTextCursor resultCursor(docHandle(), startPos);
    resultCursor.setPosition(endPos, QTextCursor::KeepAnchor);
    return resultCursor;
  } else {
    return QTextCursor();
  }
}

QTextCursor Document::find(const Regexp* expr,
                           const QTextCursor& cursor,
                           int begin,
                           int end,
                           Document::FindFlags options) const {
  if (cursor.isNull())
    return QTextCursor();

  int pos;
  if (options & QTextDocument::FindBackward) {
    pos = cursor.selectionStart();
  } else {
    pos = cursor.selectionEnd();
  }
  return find(expr, pos, begin, end, options);
}
