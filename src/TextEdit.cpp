#include <tuple>
#include <string>
#include <algorithm>
#include <QtWidgets>

#include "TextEdit_p.h"
#include "LineNumberArea.h"
#include "core/TextEditLogic.h"
#include "KeymapManager.h"
#include "CommandManager.h"
#include "OpenRecentItemManager.h"
#include "DocumentManager.h"
#include "core/Config.h"
#include "Helper.h"
#include "App.h"
#include "Window.h"
#include "core/Metadata.h"
#include "core/LanguageParser.h"
#include "core/Theme.h"
#include "core/Constants.h"
#include "core/BOM.h"
#include "core/Region.h"
#include "core/Util.h"
#include "core/TextCursor.h"

using core::Document;
using core::Encoding;
using core::Language;
using core::Region;
using core::Metadata;
using core::Config;
using core::TextEditLogic;
using core::Theme;
using core::ColorSettings;
using core::Regexp;
using core::Constants;
using core::BOM;
using core::Region;
using core::Util;
using core::TextCursor;

namespace {
const QString DEFAULT_SCOPE = "text.plain";

QString preservedCaseText(const QString& oldStr, const QString& newStr) {
  if (oldStr.isEmpty()) {
    return newStr;
  }

  QString resultStr;
  int oldStrIndex;
  for (int i = 0; i < newStr.size(); i++) {
    if (i > oldStr.size() - 1) {
      oldStrIndex = oldStr.size() - 1;
    } else {
      oldStrIndex = i;
    }

    if (oldStr[oldStrIndex].isUpper()) {
      resultStr = resultStr + newStr[i].toUpper();
    } else {
      resultStr = resultStr + newStr[i].toLower();
    }
  }

  return resultStr;
}

void insertText(QTextCursor& cursor, const QString& text, bool preserveCase) {
  if (preserveCase) {
    cursor.insertText(preservedCaseText(cursor.selectedText(), text));
  } else {
    cursor.insertText(text);
  }
}

QColor adjustForEOL(QColor const color, int threshold = 125) {
  QColor newColor;

  // 0 is black; 255 is as far from black as possible.
  if (color.value() < threshold) {
    newColor = QColor::fromHsv(color.hue(), color.saturation(), qMax(color.value() - 30, 0));
  } else {
    newColor = QColor::fromHsv(color.hue(), color.saturation(), qMin(color.value() + 30, 255));
  }
  newColor.setAlpha(128);
  return newColor;
}
}

void TextEditPrivate::updateLineNumberAreaWidth(int /* newBlockCount */) {
  //  qDebug("updateLineNumberAreaWidth");
  q_ptr->setViewportMargins(q_ptr->lineNumberAreaWidth(), 0, 0, 0);
}

void TextEditPrivate::updateLineNumberArea(const QRect& rect, int dy) {
  //  qDebug("updateLineNumberArea");
  if (dy)
    m_lineNumberArea->scroll(0, dy);
  else
    m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

  if (rect.contains(q_ptr->viewport()->rect()))
    updateLineNumberAreaWidth(0);
}

void TextEditPrivate::setTheme(Theme* theme) {
  qDebug("TextEdit theme is changed");
  if (!theme) {
    qWarning("theme is null");
    return;
  }

  if (theme->textEditSettings) {
    const QString& style =
        QStringLiteral(
            "QPlainTextEdit {"
            "color: %1;"
            "background-color: %2;"
            "selection-color:%3;"
            "selection-background-color: %4;"
            "}")
            .arg(Util::qcolorForStyleSheet(theme->textEditSettings->value("foreground")))
            .arg(Util::qcolorForStyleSheet(theme->textEditSettings->value("background")))
            .arg(Util::qcolorForStyleSheet(theme->textEditSettings->value("selectionForeground")))
            .arg(Util::qcolorForStyleSheet(theme->textEditSettings->value("selectionBackground")));

    q_ptr->setStyleSheet(style);

    q_ptr->verticalScrollBar()->setStyleSheet(theme->textEditVerticalScrollBarStyle());
    q_ptr->horizontalScrollBar()->setStyleSheet(theme->textEditHorizontalScrollBarStyle());
  }

  highlightCurrentLine();
}

void TextEditPrivate::clearDirtyMarker() {
  q_ptr->document()->setModified(false);
}

void TextEditPrivate::clearHighlightingCurrentLine() {
  q_ptr->setExtraSelections(QList<QTextEdit::ExtraSelection>());
}

/**
 * @brief Get previous line which doesn't match the pattern
 * @param prevCount
 * @param pattern
 * @return
 */
QString TextEditPrivate::prevLineText(int prevCount, Regexp* ignorePattern) {
  auto cursor = q_ptr->textCursor();
  for (int i = 0; i < prevCount; i++) {
    bool moved = false;
    do {
      moved = cursor.movePosition(QTextCursor::PreviousBlock);
      if (!moved)
        return "";
      cursor.select(QTextCursor::LineUnderCursor);
    } while (ignorePattern && ignorePattern->matches(cursor.selectedText()));
  }

  return cursor.selectedText();
}

void TextEditPrivate::toggleHighlightingCurrentLine(bool hasSelection) {
  if (hasSelection) {
    clearHighlightingCurrentLine();
  } else {
    highlightCurrentLine();
  }
}

void TextEditPrivate::emitLanguageChanged(const QString& scope) {
  emit q_ptr->languageChanged(scope);
}

void TextEditPrivate::emitEncodingChanged(const Encoding& enc) {
  emit q_ptr->encodingChanged(enc);
}

void TextEditPrivate::emitLineSeparatorChanged(const QString& lineSeparator) {
  emit q_ptr->lineSeparatorChanged(lineSeparator);
}

void TextEditPrivate::emitBOMChanged(const core::BOM& bom) {
  emit q_ptr->bomChanged(bom);
}

void TextEditPrivate::setWordWrap(bool wordWrap) {
  if (wordWrap) {
    q_ptr->setWordWrapMode(QTextOption::WordWrap);
  } else {
    q_ptr->setWordWrapMode(QTextOption::NoWrap);
  }
}

void TextEditPrivate::setupConnections(std::shared_ptr<core::Document> document) {
  Q_Q(TextEdit);

  if (m_document) {
    // QObject::disconnect from old document
    QObject::disconnect(m_document.get(), &Document::pathUpdated, q, &TextEdit::pathUpdated);
    QObject::disconnect(m_document.get(), &Document::languageChanged, q,
                        &TextEdit::languageChanged);
    QObject::disconnect(m_document.get(), &Document::encodingChanged, q,
                        &TextEdit::encodingChanged);
    QObject::disconnect(m_document.get(), &Document::lineSeparatorChanged, q,
                        &TextEdit::lineSeparatorChanged);
    QObject::disconnect(m_document.get(), &Document::bomChanged, q, &TextEdit::bomChanged);
    QObject::disconnect(m_document.get(), SIGNAL(contentsChanged()), q,
                        SLOT(outdentCurrentLineIfNecessary()));
  }

  m_document = document;
  QObject::connect(m_document.get(), &Document::pathUpdated, q, &TextEdit::pathUpdated);
  QObject::connect(m_document.get(), &Document::languageChanged, q, &TextEdit::languageChanged);
  QObject::connect(m_document.get(), &Document::encodingChanged, q, &TextEdit::encodingChanged);
  QObject::connect(m_document.get(), &Document::lineSeparatorChanged, q,
                   &TextEdit::lineSeparatorChanged);
  QObject::connect(m_document.get(), &Document::bomChanged, q, &TextEdit::bomChanged);
  QObject::connect(m_document.get(), SIGNAL(contentsChanged()), q,
                   SLOT(outdentCurrentLineIfNecessary()));
}

boost::optional<Region> TextEditPrivate::find(const QString& text,
                                              int from,
                                              int begin,
                                              int end,
                                              Document::FindFlags flags) {
  if (text.isEmpty())
    return boost::none;

  auto isBackward = flags.testFlag(Document::FindFlag::FindBackward);

  if (Document* doc = q_ptr->document()) {
    auto maybeRegion = doc->find(text, from, begin, end, flags);
    if (maybeRegion) {
      QTextCursor resultCursor = QTextCursor(doc->docHandle(), maybeRegion->begin());
      resultCursor.setPosition(maybeRegion->end(), QTextCursor::KeepAnchor);
      int oldScrollValue = q_ptr->verticalScrollBar()->value();
      q_ptr->setTextCursor(resultCursor);
      if (q_ptr->verticalScrollBar()->value() != oldScrollValue) {
        q_ptr->centerCursor();
      }
      return maybeRegion;
    } else {
      // try to find from the end of file when backward or the beginning of file.
      QTextCursor nextFindCursor(doc);
      if (isBackward) {
        if (end < 0) {
          nextFindCursor.movePosition(QTextCursor::End);
          from = nextFindCursor.position();
        } else {
          nextFindCursor.setPosition(end);
          from = end;
        }
      } else {
        nextFindCursor.setPosition(begin);
        from = begin;
      }
      maybeRegion = doc->find(text, from, begin, end, flags);
      if (maybeRegion) {
        QTextCursor resultCursor = QTextCursor(doc->docHandle(), maybeRegion->begin());
        resultCursor.setPosition(maybeRegion->end(), QTextCursor::KeepAnchor);
        int oldScrollValue = q_ptr->verticalScrollBar()->value();
        q_ptr->setTextCursor(resultCursor);
        if (q_ptr->verticalScrollBar()->value() != oldScrollValue) {
          q_ptr->centerCursor();
        }
        return maybeRegion;
      }
    }
  }

  return boost::none;
}

void TextEditPrivate::highlightCurrentLine() {
  if (q_ptr->textCursor().hasSelection()) {
    return;
  }
  Theme* theme = Config::singleton().theme();
  if (theme->textEditSettings != nullptr) {
    ColorSettings* textEditSettings = theme->textEditSettings.get();

    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!q_ptr->isReadOnly()) {
      QTextEdit::ExtraSelection selection;
      QColor lineColor = QColor(textEditSettings->value("lineHighlight"));
      selection.format.setBackground(lineColor);
      selection.format.setProperty(QTextFormat::FullWidthSelection, true);
      selection.cursor = q_ptr->textCursor();
      selection.cursor.clearSelection();
      extraSelections.append(selection);
    }
    q_ptr->setExtraSelections(extraSelections);
  }
}

/**
 * @brief Indent one level
 * @param currentVisibleCursor
 */
void TextEditPrivate::indentOneLevel(QTextCursor& currentVisibleCursor) {
  TextEditLogic::indentOneLevel(currentVisibleCursor, Config::singleton().indentUsingSpaces(),
                                tabWidth());
}

int TextEditPrivate::tabWidth() {
  return m_document ? m_document->tabWidth(m_document->language()) : Config::singleton().tabWidth();
}

void TextEditPrivate::outdentOneLevel(QTextCursor& currentVisibleCursor) {
  TextEditLogic::outdent(m_document.get(), currentVisibleCursor, tabWidth());
}

/**
 * @brief Outdent one level
 * @param currentVisibleCursor
 */
TextEditPrivate::TextEditPrivate(TextEdit* textEdit) : q_ptr(textEdit), m_document(nullptr) {}

void TextEditPrivate::outdentCurrentLineIfNecessary() {
  if (!m_document || !m_document->language()) {
    return;
  }
  auto metadata = Metadata::get(m_document->language()->scopeName);
  if (!metadata) {
    return;
  }

  auto currentVisibleCursor = q_ptr->textCursor();
  const QString& currentLineText = m_document->findBlock(currentVisibleCursor.position()).text();
  const QString& prevLineString = prevLineText();
  if (TextEditLogic::isOutdentNecessary(
          metadata->increaseIndentPattern(), metadata->decreaseIndentPattern(), currentLineText,
          prevLineString, currentVisibleCursor.atBlockEnd(), tabWidth())) {
    TextEditLogic::outdent(m_document.get(), currentVisibleCursor, tabWidth());
  }
}

TextEdit::TextEdit(QWidget* parent)
    : QPlainTextEdit(parent), d_ptr(new TextEditPrivate(this)), m_showLineNumber(true) {
  d_ptr->m_lineNumberArea = new LineNumberArea(this);
  d_ptr->setWordWrap(Config::singleton().wordWrap());

  Q_D(TextEdit);
  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(const QRect&, int)), this,
          SLOT(updateLineNumberArea(const QRect&, int)));
  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
  connect(this, SIGNAL(showLineNumberChanged(bool)), this, SLOT(update()));
  connect(this, &TextEdit::destroying, &OpenRecentItemManager::singleton(),
          &OpenRecentItemManager::addOpenRecentItem);
  // can't use SIGNAL/SLOT syntax because method signature is different (doesn't consider
  // namespace).
  // Config::themeChanged(Theme*) but TextEdit::setTheme(core::Theme*)
  connect(&Config::singleton(), &Config::themeChanged, this, &TextEdit::setTheme);
  connect(&Config::singleton(), &Config::showInvisiblesChanged, this,
          static_cast<void (QWidget::*)()>(&QWidget::update));
  connect(&Config::singleton(), &Config::endOfLineStrChanged, this,
          [=](const QString&) { update(); });
  connect(this, SIGNAL(saved()), this, SLOT(clearDirtyMarker()));
  connect(this, SIGNAL(copyAvailable(bool)), this, SLOT(toggleHighlightingCurrentLine(bool)));
  connect(&Config::singleton(), SIGNAL(wordWrapChanged(bool)), this, SLOT(setWordWrap(bool)));

  // Set default values
  d->updateLineNumberAreaWidth(0);
  d_ptr->setTheme(Config::singleton().theme());
  QApplication::setCursorFlashTime(0);
  setLanguage(DEFAULT_SCOPE);
}

TextEdit::~TextEdit() {
  if (d_ptr->m_document) {
    emit destroying(d_ptr->m_document->path(), QPrivateSignal());
  }
  qDebug("~TextEdit");
}

QString TextEdit::path() {
  return d_ptr->m_document ? d_ptr->m_document->path() : QStringLiteral("");
}

void TextEdit::setTextCursor(const QTextCursor& cursor) {
  QPlainTextEdit::setTextCursor(cursor);
}

QTextCursor TextEdit::textCursor() const {
  return QPlainTextEdit::textCursor();
}

Document* TextEdit::document() {
  return d_ptr->m_document ? d_ptr->m_document.get() : nullptr;
}

QRect TextEdit::cursorRect() const {
  return QPlainTextEdit::cursorRect();
}

QRect TextEdit::cursorRect(const QTextCursor& cursor) const {
  return QPlainTextEdit::cursorRect(cursor);
}

void TextEdit::setDocument(std::shared_ptr<Document> document) {
  QPlainTextEdit::setDocument(document.get());

  Q_D(TextEdit);

  // Compare previous and current languages
  Language* prevLang = nullptr;
  Language* newLang = nullptr;
  if (d_ptr->m_document) {
    prevLang = d_ptr->m_document->language();
  }
  if (document) {
    newLang = document->language();
  }
  if (!((prevLang == newLang) ||
        (prevLang && newLang && prevLang->scopeName == newLang->scopeName))) {
    if (newLang) {
      setLanguage(newLang->scopeName);
    }
  }

  // Compare previous and current encodings
  boost::optional<Encoding> prevEnc = boost::none;
  boost::optional<Encoding> newEnc = boost::none;
  if (d_ptr->m_document) {
    prevEnc = d_ptr->m_document->encoding();
  }
  if (document) {
    newEnc = document->encoding();
  }
  if (prevEnc != newEnc && newEnc) {
    emit encodingChanged(*newEnc);
  }

  // Compare previous and current line separators
  boost::optional<QString> prevSeparator = boost::none;
  boost::optional<QString> newSeparator = boost::none;
  if (d->m_document) {
    prevSeparator = d->m_document->lineSeparator();
  }
  if (document) {
    newSeparator = document->lineSeparator();
  }
  if (prevSeparator != newSeparator && newSeparator) {
    emit lineSeparatorChanged(*newSeparator);
  }

  // Compare previous and current BOM
  boost::optional<core::BOM> prevBOM = boost::none;
  boost::optional<core::BOM> newBOM = boost::none;
  if (d_ptr->m_document) {
    prevBOM = d_ptr->m_document->bom();
  }
  if (document) {
    newBOM = document->bom();
  }
  if (prevBOM != newBOM && newBOM) {
    emit bomChanged(*newBOM);
  }

  // Compare previous and current path
  QString prevPath, newPath;
  if (d_ptr->m_document) {
    prevPath = d_ptr->m_document->path();
  }
  if (document) {
    newPath = document->path();
  }
  if (prevPath != newPath && !newPath.isEmpty()) {
    emit pathUpdated(newPath);
  }

  d->setupConnections(document);
  d->updateLineNumberAreaWidth(blockCount());

  // Special handling for user keymap.yml
  if (document->path() == Constants::singleton().userKeymapPath()) {
    connect(this, &TextEdit::saved, &KeymapManager::singleton(), &KeymapManager::loadUserKeymap);
  }
}

Language* TextEdit::language() {
  if (d_ptr->m_document) {
    return d_ptr->m_document->language();
  }
  return nullptr;
}

boost::optional<Encoding> TextEdit::encoding() {
  if (d_ptr->m_document) {
    return d_ptr->m_document->encoding();
  }
  return boost::none;
}

boost::optional<QString> TextEdit::lineSeparator() {
  if (d_ptr->m_document) {
    return d_ptr->m_document->lineSeparator();
  }
  return boost::none;
}

void TextEdit::setLineSeparator(const QString& lineSeparator) {
  if (d_ptr->m_document) {
    d_ptr->m_document->setLineSeparator(lineSeparator);
  }
}

void TextEdit::setLanguage(const QString& scopeName) {
  if (d_ptr->m_document) {
    d_ptr->m_document->setLanguage(scopeName);
  }
}

boost::optional<core::BOM> TextEdit::BOM() {
  if (d_ptr->m_document) {
    return d_ptr->m_document->bom();
  }
  return boost::none;
}

void TextEdit::setBOM(const core::BOM& bom) {
  if (d_ptr->m_document) {
    d_ptr->m_document->setBOM(bom);
  }
}

void TextEdit::setPath(const QString& path) {
  if (path.isEmpty())
    return;

  d_ptr->m_document->setPath(path);
}

boost::optional<Region> TextEdit::find(const QString& text,
                                       int from,
                                       int begin,
                                       int end,
                                       Document::FindFlags flags) {
  return d_ptr->find(text, from, begin, end, flags);
}

int TextEdit::lineNumberAreaWidth() {
  if (!m_showLineNumber) {
    return 0;
  }

  int digits = 3;
  int max = qMax(1, blockCount());
  while (max >= 1000) {
    max /= 10;
    ++digits;
  }

  int space = 10 + Config::singleton().fontMetrics().width(QLatin1Char('9')) * digits;

  return space;
}

void TextEdit::resizeEvent(QResizeEvent* e) {
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  d_ptr->m_lineNumberArea->setGeometry(
      QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextEdit::lineNumberAreaPaintEvent(QPaintEvent* event) {
  if (!m_showLineNumber) {
    return;
  }

  QPainter painter(d_ptr->m_lineNumberArea);
  painter.fillRect(event->rect(), d_ptr->m_lineNumberArea->backgroundColor());

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
  int bottom = top + (int)blockBoundingRect(block).height();

  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      QString number = QString::number(blockNumber + 1);
      painter.setFont(Config::singleton().font());
      painter.setPen(d_ptr->m_lineNumberArea->lineNumberColor());
      painter.drawText(0, top,
                       d_ptr->m_lineNumberArea->width() - d_ptr->m_lineNumberArea->PADDING_RIGHT,
                       Config::singleton().fontMetrics().height(), Qt::AlignRight, number);
    }

    block = block.next();
    top = bottom;
    bottom = top + (int)blockBoundingRect(block).height();
    ++blockNumber;
  }
}

void TextEdit::paintEvent(QPaintEvent* e) {
  QPlainTextEdit::paintEvent(e);

  QPainter painter(viewport());

  // highlight search matched texts
  foreach (const Region& region, d_ptr->m_searchMatchedRegions) {
    QTextCursor beginCursor(document()->docHandle(), region.begin());
    QTextCursor endCursor(document()->docHandle(), region.end());
    int beginPos = beginCursor.positionInBlock();
    QTextBlock block = beginCursor.block();
    QTextBlock endBlock = endCursor.block();
    do {
      int endPos =
          block == endBlock ? endCursor.positionInBlock() : block.position() + block.length();
      QTextLine textLine = block.layout()->lineForTextPosition(beginPos);
      // textLine is invalid when the character at beginPos is a new line
      if (textLine.isValid()) {
        QRectF lineRect = textLine.naturalTextRect();
        lineRect.setLeft(textLine.cursorToX(beginPos));
        lineRect.setRight(textLine.cursorToX(endPos));
        lineRect = lineRect.translated(blockBoundingGeometry(block).topLeft() + contentOffset());
        painter.drawRoundedRect(lineRect, 3.0, 3.0);
      }

      if (block != endBlock) {
        block = block.next();
        beginPos = 0;
      } else {
        break;
      }
    } while (true);
  }

  if (Config::singleton().showInvisibles()) {
    // draw an EOL string
    const int bottom = viewport()->rect().height();

    QColor invisibleColor = adjustForEOL(palette().foreground().color());
    painter.setPen(invisibleColor);

    QTextCursor cur = textCursor();
    cur.movePosition(QTextCursor::End);
    const int posEOF = cur.position();
    QTextBlock block = firstVisibleBlock();
    while (block.isValid()) {
      cur.setPosition(block.position());
      cur.movePosition(QTextCursor::EndOfBlock);
      if (cur.position() == posEOF) {
        break;
      }
      QRect r = cursorRect(cur);
      if (r.top() >= bottom)
        break;
      if (!Config::singleton().endOfLineStr().isEmpty()) {
        painter.drawText(QPointF(r.left(), r.bottom()), Config::singleton().endOfLineStr());
      }
      block = block.next();
    }
    cur.movePosition(QTextCursor::End);
    QRect r = cursorRect(cur);

    // draw an end of file string
    if (!Config::singleton().endOfFileStr().isEmpty()) {
      painter.drawText(QPointF(r.left(), r.bottom()), Config::singleton().endOfFileStr());
    }
  }
}

void TextEdit::setFontPointSize(int sz) {
  QFont ft = font();
  ft.setPointSize(sz);
  setFont(ft);
}

void TextEdit::makeFontBigger(bool bigger) {
  int sz = font().pointSize();
  if (bigger) {
    ++sz;
  } else if (!--sz)
    return;
  setFontPointSize(sz);
}

void TextEdit::dropEvent(QDropEvent* e) {
  if (e->mimeData()->hasUrls()) {
    for (const QUrl& url : e->mimeData()->urls()) {
      emit fileDropped(url.toLocalFile());
    }
  }
}

void TextEdit::timerEvent(QTimerEvent* event) {
  if (event->timerId() == trippleClickTimer.timerId()) {
    trippleClickTimer.stop();
  }
  QPlainTextEdit::timerEvent(event);
}

void TextEdit::setViewportMargins(int left, int top, int right, int bottom) {
  QPlainTextEdit::setViewportMargins(left, top, right, bottom);
}

void TextEdit::setTheme(core::Theme* theme) {
  Q_D(TextEdit);
  d->setTheme(theme);
}

int TextEdit::highlightSearchMatches(const QString& text,
                                      int begin,
                                      int end,
                                      Document::FindFlags flags) {
  d_ptr->m_searchMatchedRegions.clear();

  auto regions = document()->findAll(text, begin, end, flags);
  for (const auto& region : regions) {
    d_ptr->m_searchMatchedRegions.append(region);
  }
  update();
  return regions.size();
}

void TextEdit::clearSearchHighlight() {
  d_ptr->m_searchMatchedRegions.clear();
  update();
}

void TextEdit::replaceSelection(const QString& text, bool preserveCase) {
  QTextCursor cursor = textCursor();
  if (cursor.hasSelection()) {
    cursor.beginEditBlock();
    insertText(cursor, text, preserveCase);
    cursor.endEditBlock();
  }
}

void TextEdit::replaceAllSelection(const QString& findText,
                                   const QString& replaceText,
                                   int begin,
                                   int end,
                                   Document::FindFlags flags,
                                   bool preserveCase) {
  if (Document* doc = document()) {
    QTextCursor currentCursor = textCursor();
    currentCursor.beginEditBlock();

    int delta = 0;
    for (const auto& region : doc->findAll(findText, begin, end, flags)) {
      QTextCursor cursor(doc->docHandle(), region.begin() + delta);
      cursor.setPosition(region.end() + delta, QTextCursor::KeepAnchor);
      delta += (replaceText.size() - cursor.selectedText().size());
      insertText(cursor, replaceText, preserveCase);
    }

    currentCursor.endEditBlock();
    clearSearchHighlight();
    update();
  }
}

void TextEdit::insertNewLine() {
  // textCursor()->insertBlock() doesn't work because QPlainTextEdit does more things
  // than just inserting a new block such as ensuring a cursor visible.
  // Instead, we send return key press event directly to QPlainTextEdit.
  // see qwidgettextcontrol.cpp at 1372
  QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
  QPlainTextEdit::keyPressEvent(&event);

  // Indent a new line based on indent settings
  QString prevLineString = "";
  Metadata* metadata = nullptr;
  if (d_ptr->m_document && d_ptr->m_document->language()) {
    metadata = Metadata::get(d_ptr->m_document->language()->scopeName);
    if (metadata) {
      prevLineString = d_ptr->prevLineText(1, metadata->unIndentedLinePattern());
    }
  }
  if (!metadata) {
    prevLineString = d_ptr->prevLineText();
  }

  if (prevLineString.isEmpty()) {
    return;
  }

  boost::optional<QString> prevPrevLineText;
  if (metadata) {
    prevPrevLineText = d_ptr->prevLineText(2, metadata->unIndentedLinePattern());
  } else {
    prevPrevLineText = boost::none;
  }
  bool indentUsingSpaces = Config::singleton().indentUsingSpaces();
  auto cursor = textCursor();
  TextEditLogic::indentCurrentLine(d_ptr->m_document.get(), cursor, prevLineString,
                                   prevPrevLineText, metadata, indentUsingSpaces,
                                   d_ptr->tabWidth());
}

TextEdit* TextEdit::clone() {
  TextEdit* textEdit = new TextEdit(this);
  textEdit->setDocument(d_ptr->m_document);
  return textEdit;
}

void TextEdit::save() {
  save(false);
}

void TextEdit::saveAs() {
  QString newFilePath = DocumentManager::singleton().saveAs(d_ptr->m_document.get(), false);
  if (!newFilePath.isEmpty()) {
    setPath(newFilePath);
    emit saved();
  }
}

void TextEdit::deleteChar(int repeat) {
  QTextCursor cur = textCursor();
  if (!cur.hasSelection()) {
    const int pos = cur.position();
    int dst;
    if (repeat > 0) {
      cur.movePosition(QTextCursor::EndOfBlock);
      const int endpos = cur.position();
      if (pos == endpos)
        return;
      dst = qMin(pos + repeat, endpos);
      cur.setPosition(pos);
    } else {
      const int blockPos = cur.block().position();
      if (pos == blockPos)
        return;
      dst = qMax(pos + repeat, blockPos);
    }

    cur.setPosition(dst, QTextCursor::KeepAnchor);
  }

  cur.deleteChar();
}

bool TextEdit::isThinCursor() {
  return !overwriteMode();
}

void TextEdit::setThinCursor(bool on) {
  setOverwriteMode(!on);
  update();
}

void TextEdit::wheelEvent(QWheelEvent* e) {
  Qt::KeyboardModifiers mod = e->modifiers();
  if ((mod & Qt::ControlModifier) != 0) {
    makeFontBigger(e->delta() > 0);
  } else {
    QPlainTextEdit::wheelEvent(e);
  }
}

void TextEdit::keyPressEvent(QKeyEvent* event) {
  if (QApplication::activePopupWidget()) {
    switch (event->key()) {
      case Qt::Key_Up:
      case Qt::Key_Down:
      case Qt::Key_Enter:
      case Qt::Key_Return:
      case Qt::Key_Escape:
      case Qt::Key_Control:
      case Qt::Key_Meta:
      case Qt::Key_Shift:
      case Qt::Key_Alt:
        event->ignore();
        return;
      default:
        QApplication::activePopupWidget()->hide();
        break;
    }
  }

  // Override default word movement shortcuts defined in QWidgetTextControl
  if (event == QKeySequence::MoveToNextWord) {
    auto cursor = textCursor();
    TextCursor::customMovePosition(cursor, QTextCursor::NextWord, QTextCursor::MoveAnchor);
    setTextCursor(cursor);
    event->accept();
    return;
  } else if (event == QKeySequence::MoveToPreviousWord) {
    auto cursor = textCursor();
    TextCursor::customMovePosition(cursor, QTextCursor::PreviousWord, QTextCursor::MoveAnchor);
    setTextCursor(cursor);
    event->accept();
    return;
  } else if (event == QKeySequence::SelectNextWord) {
    auto cursor = textCursor();
    TextCursor::customMovePosition(cursor, QTextCursor::NextWord, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
    event->accept();
    return;
  } else if (event == QKeySequence::SelectPreviousWord) {
    auto cursor = textCursor();
    TextCursor::customMovePosition(cursor, QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
    event->accept();
    return;
  } else if (event == QKeySequence::DeleteEndOfWord) {
    auto cursor = textCursor();
    TextCursor::customMovePosition(cursor, QTextCursor::NextWord, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    event->accept();
    return;
  } else if (event == QKeySequence::DeleteStartOfWord) {
    auto cursor = textCursor();
    TextCursor::customMovePosition(cursor, QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    event->accept();
    return;
  }

  switch (event->key()) {
    // Override QPlainTextEdit default behavior
    case Qt::Key_Home: {
      auto cursor = textCursor();
      auto moveMode = QTextCursor::MoveMode::MoveAnchor;
      if (event->modifiers() & Qt::ShiftModifier) {
        moveMode = QTextCursor::MoveMode::KeepAnchor;
      }
      cursor.movePosition(QTextCursor::MoveOperation::StartOfLine, moveMode);
      setTextCursor(cursor);
      event->accept();
      return;
    }
    case Qt::Key_End: {
      auto cursor = textCursor();
      auto moveMode = QTextCursor::MoveMode::MoveAnchor;
      if (event->modifiers() & Qt::ShiftModifier) {
        moveMode = QTextCursor::MoveMode::KeepAnchor;
      }
      cursor.movePosition(QTextCursor::MoveOperation::EndOfLine, moveMode);
      setTextCursor(cursor);
      event->accept();
      return;
    }
  }

  QPlainTextEdit::keyPressEvent(event);
}

void TextEdit::mousePressEvent(QMouseEvent* event) {
  if (trippleClickTimer.isActive() &&
      ((event->pos() - trippleClickPoint).manhattanLength() < QApplication::startDragDistance())) {
    trippleClickTimer.stop();

    auto cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
    event->accept();
    return;
  }

  QPlainTextEdit::mousePressEvent(event);
}

void TextEdit::mouseDoubleClickEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton && textInteractionFlags() & Qt::TextSelectableByMouse) {
    auto cursor = textCursor();
    TextCursor::customSelect(cursor, QTextCursor::WordUnderCursor);
    setTextCursor(cursor);
    event->accept();

    trippleClickPoint = event->pos();
    trippleClickTimer.start(QApplication::doubleClickInterval(), this);

    return;
  }
  QPlainTextEdit::mouseDoubleClickEvent(event);
}

void TextEdit::clearSelection() {
  QTextCursor cursor = textCursor();
  if (cursor.hasSelection()) {
    cursor.clearSelection();
    setTextCursor(cursor);
  }
}

void TextEdit::save(bool beforeClose) {
  if (DocumentManager::singleton().save(d_ptr->m_document.get(), beforeClose)) {
    emit saved();
  }
}

void TextEdit::saveState(QSettings& settings) {
  Q_D(TextEdit);
  settings.beginGroup(TextEdit::staticMetaObject.className());
  if (d->m_document) {
    d->m_document->saveState(settings);
  }
  settings.endGroup();
}

void TextEdit::loadState(QSettings& settings) {
  settings.beginGroup(TextEdit::staticMetaObject.className());

  if (settings.childGroups().contains(Document::SETTINGS_PREFIX)) {
    auto doc = DocumentManager::singleton().getOrCreate(settings);
    setDocument(doc);
  } else {
    setDocument(std::shared_ptr<Document>(Document::createBlank()));
  }

  settings.endGroup();
}

bool TextEdit::isSearchMatchesHighlighted() {
  return !d_ptr->m_searchMatchedRegions.isEmpty();
}

QString TextEdit::scopeName() {
  Q_D(TextEdit);
  return d->m_document->scopeName(textCursor().position());
}

QString TextEdit::scopeTree() {
  Q_D(TextEdit);
  return d->m_document->scopeTree();
}

void TextEdit::undo() {
  QPlainTextEdit::undo();
}

void TextEdit::redo() {
  QPlainTextEdit::redo();
}

void TextEdit::cut() {
  QPlainTextEdit::cut();
}

void TextEdit::copy() {
  QPlainTextEdit::copy();
}

void TextEdit::paste() {
  QPlainTextEdit::paste();
}

void TextEdit::selectAll() {
  QPlainTextEdit::selectAll();
}

void TextEdit::indent() {
  Q_D(TextEdit);
  auto cursor = textCursor();
  d->indentOneLevel(cursor);
}

void TextEdit::outdent() {
  Q_D(TextEdit);
  auto cursor = textCursor();
  d->outdentOneLevel(cursor);
}

QString TextEdit::toText() {
  return QPlainTextEdit::toPlainText();
}

void TextEdit::setText(const QString& text) {
  QPlainTextEdit::setPlainText(text);
}

// Necessary for Q_PRIVATE_SLOT
#include "moc_TextEdit.cpp"
