#include <boost/optional.hpp>
#include <tuple>
#include <QPlainTextDocumentLayout>
#include <QTextCodec>

#include "core/Document.h"
#include "core/LineSeparator.h"
#include "Session.h"
#include "core/LanguageParser.h"
#include "core/Regexp.h"

namespace {
using core::Encoding;
using core::LineSeparator;

boost::optional<std::tuple<QString, Encoding, QString>> load(const QString& path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadWrite))
    return boost::none;

  QByteArray contentBytes = file.readAll();
  QTextCodec* codec;
  auto guessedEncoding = Encoding::guessEncoding(contentBytes);
  codec = guessedEncoding.codec();
  const QString& text = codec->toUnicode(contentBytes);
  const LineSeparator& separator = LineSeparator::guess(text);
  return std::make_tuple(text, guessedEncoding, separator.separatorStr());
}

boost::optional<std::tuple<QString, QString>> load(const QString& path, const Encoding& encoding) {
  QFile file(path);
  if (!file.open(QIODevice::ReadWrite))
    return boost::none;

  QByteArray contentBytes = file.readAll();
  QTextCodec* codec = encoding.codec();
  const QString& text = codec->toUnicode(contentBytes);
  return std::make_tuple(text, LineSeparator::guess(text).separatorStr());
}
}

namespace core {

Document::Document(const QString& path,
                   const QString& text,
                   const Encoding& encoding,
                   const QString& separator)
    : QTextDocument(text),
      m_path(path),
      m_encoding(encoding),
      m_lineSeparator(separator),
      m_syntaxHighlighter(nullptr) {
  setDefaultFont(Session::singleton().font());
  setupLayout();

  int dotPos = path.lastIndexOf('.');
  if (dotPos >= 0) {
    QString ext = path.mid(dotPos + 1);
    qDebug("ext: %s", qPrintable(ext));
    setupSyntaxHighlighter(LanguageProvider::languageFromExtension(ext), toPlainText());
  } else {
    qDebug("extension not found. path: %s", qPrintable(path));
  }

  connect(&Session::singleton(), &Session::fontChanged, this, &QTextDocument::setDefaultFont);
}

Document::Document()
    : m_encoding(Encoding::defaultEncoding()),
      m_lineSeparator(LineSeparator::defaultLineSeparator().separatorStr()),
      m_syntaxHighlighter(nullptr) {
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
  if (const boost::optional<std::tuple<QString, Encoding, QString>> textAndEncAndSeparator =
          load(path)) {
    return new Document(path, std::get<0>(*textAndEncAndSeparator),
                        std::get<1>(*textAndEncAndSeparator), std::get<2>(*textAndEncAndSeparator));
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

void Document::setLanguage(const QString& scopeName) {
  qDebug("setLanguage: %s", qPrintable(scopeName));
  Language* newLang = LanguageProvider::languageFromScope(scopeName);
  if (m_lang.get() == newLang || (m_lang && newLang && *m_lang == *newLang)) {
    qDebug("lang is already %s", qPrintable(scopeName));
    return;
  }

  m_lang.reset(newLang);
  if (m_lang && m_syntaxHighlighter) {
    LanguageParser* parser = LanguageParser::create(m_lang->scopeName, toPlainText());
    m_syntaxHighlighter->setParser(parser);
    if (m_syntaxHighlighter) {
      m_syntaxHighlighter->rehighlight();
    }
  }
  emit languageChanged(scopeName);
}

void Document::setEncoding(const Encoding& encoding) {
  if (m_encoding != encoding) {
    m_encoding = encoding;
    emit encodingChanged(encoding);
  }
}

void Document::setLineSeparator(const QString& lineSeparator) {
  if (m_lineSeparator != lineSeparator) {
    m_lineSeparator = lineSeparator;
    emit lineSeparatorChanged(lineSeparator);
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

void Document::reload() {
  if (const boost::optional<std::tuple<QString, Encoding, QString>> textAndEncAndSeparator =
          load(m_path)) {
    setPlainText(std::get<0>(*textAndEncAndSeparator));
    setEncoding(std::get<1>(*textAndEncAndSeparator));
    setLineSeparator(std::get<2>(*textAndEncAndSeparator));
    setModified(false);
    emit modificationChanged(false);
  }
}

void Document::reload(const Encoding& encoding) {
  if (const boost::optional<std::tuple<QString, QString>> textAndSeparator =
          load(m_path, encoding)) {
    setPlainText(std::get<0>(*textAndSeparator));
    setEncoding(encoding);
    setLineSeparator(std::get<1>(*textAndSeparator));
    setModified(false);
    emit modificationChanged(false);
  }
}

}  // namespace core
