#include <boost/optional.hpp>
#include <tuple>
#include <QPlainTextDocumentLayout>
#include <QTextCodec>

#include "Document.h"

namespace {
boost::optional<std::tuple<QString, Encoding>> load(const QString& path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadWrite))
    return boost::none;

  QByteArray contentBytes = file.readAll();
  QTextCodec* codec;
  auto guessedEncoding = Encoding::guessEncoding(contentBytes);
  codec = guessedEncoding.codec();
  return std::make_tuple(codec->toUnicode(contentBytes), guessedEncoding);
}

boost::optional<QString> load(const QString& path, const Encoding& encoding) {
  QFile file(path);
  if (!file.open(QIODevice::ReadWrite))
    return boost::none;

  QByteArray contentBytes = file.readAll();
  QTextCodec* codec = encoding.codec();
  return codec->toUnicode(contentBytes);
}
}

Document::Document(const QString& path, const QString& text, const Encoding& encoding)
    : QTextDocument(text), m_path(path), m_encoding(encoding), m_syntaxHighlighter(nullptr) {
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

Document::Document() : m_encoding(Encoding::defaultEncoding()), m_syntaxHighlighter(nullptr) {
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

Document::~Document() {
  qDebug("~Document");
}

Document* Document::create(const QString& path) {
  qDebug("Docment::create(%s)", qPrintable(path));
  if (const boost::optional<std::tuple<QString, Encoding>> textAndEnc = load(path)) {
    return new Document(path, std::get<0>(*textAndEnc), std::get<1>(*textAndEnc));
  } else {
    return nullptr;
  }
}

Document* Document::createBlank() {
  return new Document();
}

void Document::setPath(const QString& path) {
  m_path = path;
  emit pathUpdated(path);
}

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

void Document::setEncoding(const Encoding& encoding) {
  if (m_encoding != encoding) {
    m_encoding = encoding;
  }
}

QTextCursor Document::find(const QString& subString,
                           int from,
                           int begin,
                           int end,
                           Document::FindFlags options) const {
  if (subString.isEmpty()) {
    return QTextCursor();
  }

  bool isCaseSensitive = options & FindFlag::FindCaseSensitively;
  bool isRegex = options & FindFlag::FindRegex;
  bool isWholeWord = options & FindFlag::FindWholeWords;

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
  if (options & FindFlag::FindBackward) {
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
  bool isBackward = options.testFlag(FindFlag::FindBackward);
  qDebug("find: %s, back: %d, from: %d, begin: %d, end: %d", qPrintable(expr->pattern()),
         (options.testFlag(FindFlag::FindBackward)), from, begin, end);
  QString str = toPlainText();
  QStringRef text = isBackward ? str.midRef(begin, from - begin) : str.midRef(from, end - from);
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

QString Document::scopeName(int pos) const {
  return m_syntaxHighlighter ? m_syntaxHighlighter->scopeName(pos) : "";
}

QString Document::scopeTree() const {
  return m_syntaxHighlighter ? m_syntaxHighlighter->scopeTree() : "";
}

void Document::reload(const Encoding& encoding) {
  if (const boost::optional<QString> text = load(m_path, encoding)) {
    setPlainText(*text);
    setEncoding(encoding);
    setModified(false);
  }
}
