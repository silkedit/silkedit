#include <tuple>
#include <string>
#include <algorithm>
#include <QtWidgets>
#include <QStringBuilder>

#include "vi.h"
#include "TextEditView_p.h"
#include "LineNumberArea.h"
#include "core/TextEditViewLogic.h"
#include "KeymapManager.h"
#include "CommandManager.h"
#include "OpenRecentItemManager.h"
#include "DocumentManager.h"
#include "core/Config.h"
#include "API.h"
#include "PluginManager.h"
#include "core/Metadata.h"
#include "core/LanguageParser.h"
#include "core/Theme.h"
#include "core/Constants.h"
#include "core/BOM.h"

using core::Document;
using core::Encoding;
using core::Language;
using core::Region;
using core::Metadata;
using core::Config;
using core::TextEditViewLogic;
using core::Theme;
using core::ColorSettings;
using core::Regexp;
using core::Constants;
using core::BOM;

namespace {
const QString DEFAULT_SCOPE = "text.plain";

bool caseInsensitiveLessThan(const QString& a, const QString& b) {
  return a.compare(b, Qt::CaseInsensitive) < 0;
}

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
      resultStr = resultStr % newStr[i].toUpper();
    } else {
      resultStr = resultStr % newStr[i].toLower();
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

int toMoveOperation(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  if (str == "up") {
    return QTextCursor::Up;
  } else if (str == "down") {
    return QTextCursor::Down;
  } else if (str == "left") {
    return QTextCursor::Left;
  } else if (str == "right") {
    return QTextCursor::Right;
  } else if (str == "start_of_line") {
    return QTextCursor::StartOfBlock;
  } else if (str == "first_non_blank_char") {
    return ViMoveOperation::FirstNonBlankChar;
  } else if (str == "last_char") {
    return ViMoveOperation::LastChar;
  } else if (str == "next_line") {
    return ViMoveOperation::NextLine;
  } else if (str == "prev_line") {
    return ViMoveOperation::PrevLine;
  } else {
    return QTextCursor::NoMove;
  }
}

QColor adjustForEOL(QColor const color, int threshold = 125) {
  QColor newColor;

  // 0 is black; 255 is as far from black as possible.
  if (color.value() < threshold) {
    newColor = QColor::fromHsv(color.hue(), color.saturation(), color.value() - 30);
  } else {
    newColor = QColor::fromHsv(color.hue(), color.saturation(), color.value() + 30);
  }
  newColor.setAlpha(128);
  return newColor;
}
}

void TextEditViewPrivate::updateLineNumberAreaWidth(int /* newBlockCount */) {
  //  qDebug("updateLineNumberAreaWidth");
  q_ptr->setViewportMargins(q_ptr->lineNumberAreaWidth(), 0, 0, 0);
}

void TextEditViewPrivate::updateLineNumberArea(const QRect& rect, int dy) {
  //  qDebug("updateLineNumberArea");
  if (dy)
    m_lineNumberArea->scroll(0, dy);
  else
    m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

  if (rect.contains(q_ptr->viewport()->rect()))
    updateLineNumberAreaWidth(0);
}

void TextEditViewPrivate::setTheme(Theme* theme) {
  qDebug("changeTheme");
  if (!theme) {
    qWarning("theme is null");
    return;
  }

  m_lineNumberArea->setTheme(theme);

  QString style;
  if (!theme->scopeSettings.isEmpty()) {
    ColorSettings* settings = theme->scopeSettings.first()->colorSettings.get();
    if (settings->contains("foreground")) {
      style = style % QString("color: %1;").arg(settings->value("foreground").name());
      qDebug() << QString("color: %1;").arg(settings->value("foreground").name());
    }

    if (settings->contains("background")) {
      style = style % QString("background-color: %1;").arg(settings->value("background").name());
      qDebug() << QString("background-color: %1;").arg(settings->value("background").name());
    }

    QString selectionBackgroundColor = "";
    if (settings->contains("selection")) {
      selectionBackgroundColor = settings->value("selection").name();
    } else if (settings->contains("selectionBackground")) {
      selectionBackgroundColor = settings->value("selectionBackground").name();
    }
    if (!selectionBackgroundColor.isEmpty()) {
      style = style % QString("selection-background-color: %1;").arg(selectionBackgroundColor);
      qDebug() << QString("selection-background-color: %1;")
                      .arg(settings->value("selection").name());
    }

    // for selection foreground color, we use foreground color if selectionForeground is not found.
    // The reason is that Qt ignores syntax highlighted color for a selected text and sets selection
    // foreground color something.
    // Sometimes it becomes the color hard to see. We use foreground color instead to prevent it.
    // https://bugreports.qt.io/browse/QTBUG-1344?jql=project%20%3D%20QTBUG%20AND%20text%20~%20%22QTextEdit%20selection%20color%22
    QString selectionColor = "";
    if (settings->contains("selectionForeground")) {
      selectionColor = settings->value("selectionForeground").name();
    } else if (settings->contains("foreground")) {
      selectionColor = settings->value("foreground").name();
    }

    if (!selectionColor.isEmpty()) {
      style = style % QString("selection-color: %1;").arg(selectionColor);
      qDebug() << QString("selection-color: %1;")
                      .arg(settings->value("selectionForeground").name());
    }

    q_ptr->setStyleSheet(QString("QPlainTextEdit{%1}").arg(style));
  }

  highlightCurrentLine();
}

void TextEditViewPrivate::clearDirtyMarker() {
  q_ptr->document()->setModified(false);
}

void TextEditViewPrivate::clearHighlightingCurrentLine() {
  q_ptr->setExtraSelections(QList<QTextEdit::ExtraSelection>());
}

void TextEditViewPrivate::performCompletion(const QString& completionPrefix) {
  populateModel(completionPrefix);
  if (completionPrefix != m_completer->completionPrefix()) {
    m_completer->setCompletionPrefix(completionPrefix);
    m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
  }

  if (m_completer->completionCount() == 1) {
    insertCompletion(m_completer->currentCompletion(), true);
  } else {
    QRect rect = q_ptr->cursorRect();
    rect.setWidth(m_completer->popup()->sizeHintForColumn(0) +
                  m_completer->popup()->verticalScrollBar()->sizeHint().width());
    m_completer->complete(rect);
  }
}

void TextEditViewPrivate::insertCompletion(const QString& completion) {
  insertCompletion(completion, false);
}

void TextEditViewPrivate::insertCompletion(const QString& completion, bool singleWord) {
  QTextCursor cursor = q_ptr->textCursor();
  int numberOfCharsToComplete = completion.length() - m_completer->completionPrefix().length();
  int insertionPosition = cursor.position();
  cursor.insertText(completion.right(numberOfCharsToComplete));
  if (singleWord) {
    cursor.setPosition(insertionPosition);
    cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    m_completedAndSelected = true;
  }

  q_ptr->setTextCursor(cursor);
}

void TextEditViewPrivate::populateModel(const QString& completionPrefix) {
  QStringList strings = q_ptr->toPlainText().split(QRegExp("\\W+"));
  strings.removeAll(completionPrefix);
  strings.removeDuplicates();
  qSort(strings.begin(), strings.end(), caseInsensitiveLessThan);
  m_model->setStringList(strings);
}

bool TextEditViewPrivate::handledCompletedAndSelected(QKeyEvent* event) {
  m_completedAndSelected = false;
  QTextCursor cursor = q_ptr->textCursor();
  switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      cursor.clearSelection();
      break;
    case Qt::Key_Escape:
      cursor.removeSelectedText();
      break;
    default:
      return false;
  }
  q_ptr->setTextCursor(cursor);
  event->accept();
  return true;
}

/**
 * @brief Get previous line which doesn't match the pattern
 * @param prevCount
 * @param pattern
 * @return
 */
QString TextEditViewPrivate::prevLineText(int prevCount, Regexp* ignorePattern) {
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

void TextEditViewPrivate::toggleHighlightingCurrentLine(bool hasSelection) {
  if (hasSelection) {
    clearHighlightingCurrentLine();
  } else {
    highlightCurrentLine();
  }
}

void TextEditViewPrivate::emitLanguageChanged(const QString& scope) {
  emit q_ptr->languageChanged(scope);
}

void TextEditViewPrivate::emitEncodingChanged(const Encoding& enc) {
  emit q_ptr->encodingChanged(enc);
}

void TextEditViewPrivate::emitLineSeparatorChanged(const QString& lineSeparator) {
  emit q_ptr->lineSeparatorChanged(lineSeparator);
}

void TextEditViewPrivate::emitBOMChanged(const core::BOM& bom) {
  emit q_ptr->bomChanged(bom);
}

void TextEditViewPrivate::setTabStopWidthFromSession() {
  qreal width = QFontMetricsF(Config::singleton().font()).width(QLatin1Char(' '));
  if (q_ptr->document()) {
    QTextOption option = q_ptr->document()->defaultTextOption();
    option.setTabStop(width * Config::singleton().tabWidth());
    q_ptr->document()->setDefaultTextOption(option);
  }
}

void TextEditViewPrivate::setupConnections(std::shared_ptr<core::Document> document) {
  Q_Q(TextEditView);

  if (m_document) {
    // QObject::disconnect from old document
    QObject::disconnect(m_document.get(), &Document::pathUpdated, q, &TextEditView::pathUpdated);
    QObject::disconnect(m_document.get(), &Document::languageChanged, q,
                        &TextEditView::languageChanged);
    QObject::disconnect(m_document.get(), &Document::encodingChanged, q,
                        &TextEditView::encodingChanged);
    QObject::disconnect(m_document.get(), &Document::lineSeparatorChanged, q,
                        &TextEditView::lineSeparatorChanged);
    QObject::disconnect(m_document.get(), &Document::bomChanged, q,
                        &TextEditView::bomChanged);
    QObject::disconnect(m_document.get(), SIGNAL(contentsChanged()), q,
                        SLOT(outdentCurrentLineIfNecessary()));
  }

  m_document = document;
  QObject::connect(m_document.get(), &Document::pathUpdated, q, &TextEditView::pathUpdated);
  QObject::connect(m_document.get(), &Document::languageChanged, q, &TextEditView::languageChanged);
  QObject::connect(m_document.get(), &Document::encodingChanged, q, &TextEditView::encodingChanged);
  QObject::connect(m_document.get(), &Document::lineSeparatorChanged, q,
                   &TextEditView::lineSeparatorChanged);
  QObject::connect(m_document.get(), &Document::bomChanged, q,
                   &TextEditView::bomChanged);
  QObject::connect(m_document.get(), SIGNAL(contentsChanged()), q,
                   SLOT(outdentCurrentLineIfNecessary()));
}

void TextEditViewPrivate::highlightCurrentLine() {
  if (q_ptr->textCursor().hasSelection()) {
    return;
  }

  Theme* theme = Config::singleton().theme();
  if (theme && !theme->scopeSettings.isEmpty()) {
    ColorSettings* settings = theme->scopeSettings.first()->colorSettings.get();
    if (settings->contains("lineHighlight")) {
      QList<QTextEdit::ExtraSelection> extraSelections;

      if (!q_ptr->isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(settings->value("lineHighlight"));

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = q_ptr->textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
      }

      q_ptr->setExtraSelections(extraSelections);
    } else {
      qDebug("lineHighlight not found");
    }
  } else {
    qDebug("theme is null or theme->scopeSettings is empty");
  }
}

/**
 * @brief Indent one level
 * @param currentVisibleCursor
 */
void TextEditViewPrivate::indentOneLevel(QTextCursor& currentVisibleCursor) {
  TextEditViewLogic::indentOneLevel(currentVisibleCursor, Config::singleton().indentUsingSpaces(),
                                    Config::singleton().tabWidth());
}

/**
 * @brief Outdent one level
 * @param currentVisibleCursor
 */
TextEditViewPrivate::TextEditViewPrivate(TextEditView* q_ptr)
    : q_ptr(q_ptr), m_document(nullptr), m_completedAndSelected(false) {}

void TextEditViewPrivate::outdentCurrentLineIfNecessary() {
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
  if (TextEditViewLogic::isOutdentNecessary(
          metadata->increaseIndentPattern(), metadata->decreaseIndentPattern(), currentLineText,
          prevLineString, currentVisibleCursor.atBlockEnd(), Config::singleton().tabWidth())) {
    TextEditViewLogic::outdent(m_document.get(), currentVisibleCursor,
                               Config::singleton().tabWidth());
  }
}

TextEditView::TextEditView(QWidget* parent)
    : QPlainTextEdit(parent), d_ptr(new TextEditViewPrivate(this)) {
  d_ptr->m_lineNumberArea = new LineNumberArea(this);

  Q_D(TextEditView);
  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(const QRect&, int)), this,
          SLOT(updateLineNumberArea(const QRect&, int)));
  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
  connect(this, &TextEditView::destroying, &OpenRecentItemManager::singleton(),
          &OpenRecentItemManager::addOpenRecentItem);
  // can't use SIGNAL/SLOT syntax because method signature is different (doesn't consider
  // namespace).
  // Config::themeChanged(Theme*) but TextEditView::setTheme(core::Theme*)
  connect(&Config::singleton(), &Config::themeChanged, this, &TextEditView::setTheme);
  connect(&Config::singleton(), &Config::showInvisiblesChanged, this, [=](bool) { update(); });
  connect(&Config::singleton(), &Config::endOfLineStrChanged, this,
          [=](const QString&) { update(); });
  connect(this, SIGNAL(saved()), this, SLOT(clearDirtyMarker()));
  connect(this, SIGNAL(copyAvailable(bool)), this, SLOT(toggleHighlightingCurrentLine(bool)));
  connect(&Config::singleton(), SIGNAL(fontChanged(QFont)), this,
          SLOT(setTabStopWidthFromSession()));
  connect(&Config::singleton(), SIGNAL(tabWidthChanged(int)), this,
          SLOT(setTabStopWidthFromSession()));

  d->updateLineNumberAreaWidth(0);

  QApplication::setCursorFlashTime(0);
  setLanguage(DEFAULT_SCOPE);
  d_ptr->setTheme(Config::singleton().theme());

  // setup for completion
  d_ptr->m_model.reset(new QStringListModel(this));
  d_ptr->m_completer.reset(new QCompleter(this));
  d_ptr->m_completer->setWidget(this);
  d_ptr->m_completer->setCompletionMode(QCompleter::PopupCompletion);
  d_ptr->m_completer->setModel(d_ptr->m_model.get());
  d_ptr->m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
  d_ptr->m_completer->setCaseSensitivity(Qt::CaseInsensitive);
  d_ptr->m_completer->setWrapAround(true);

  connect(d_ptr->m_completer.get(), SIGNAL(activated(const QString&)), this,
          SLOT(insertCompletion(const QString&)));
}

TextEditView::~TextEditView() {
  emit destroying(d_ptr->m_document->path());
  qDebug("~TextEditView");
}

QString TextEditView::path() {
  return d_ptr->m_document ? d_ptr->m_document->path() : "";
}

Document* TextEditView::document() {
  return d_ptr->m_document ? d_ptr->m_document.get() : nullptr;
}

void TextEditView::setDocument(std::shared_ptr<Document> document) {
  QPlainTextEdit::setDocument(document.get());

  Q_D(TextEditView);

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
  if (d_ptr->m_document && document) {
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
  if (d_ptr->m_document && document) {
    prevBOM = d_ptr->m_document->bom();
  }
  if (document) {
    newBOM = document->bom();
  }
  if (prevBOM != newBOM && newBOM) {
    emit bomChanged(*newBOM);
  }

  d->setupConnections(document);
  d->updateLineNumberAreaWidth(blockCount());
  d->setTabStopWidthFromSession();

  // Special handling for user keymap.yml
  if (document->path() == Constants::userKeymapPath()) {
    connect(this, &TextEditView::saved, &KeymapManager::singleton(),
            &KeymapManager::loadUserKeymap);
  }
}

Language* TextEditView::language() {
  if (d_ptr->m_document) {
    return d_ptr->m_document->language();
  }
  return nullptr;
}

boost::optional<Encoding> TextEditView::encoding() {
  if (d_ptr->m_document) {
    return d_ptr->m_document->encoding();
  }
  return boost::none;
}

boost::optional<QString> TextEditView::lineSeparator() {
  if (d_ptr->m_document) {
    return d_ptr->m_document->lineSeparator();
  }
  return boost::none;
}

void TextEditView::setLineSeparator(const QString& lineSeparator) {
  if (d_ptr->m_document) {
    d_ptr->m_document->setLineSeparator(lineSeparator);
  }
}

void TextEditView::setLanguage(const QString& scopeName) {
  if (d_ptr->m_document) {
    d_ptr->m_document->setLanguage(scopeName);
  }
}

boost::optional<core::BOM> TextEditView::BOM() {
  if (d_ptr->m_document) {
    return d_ptr->m_document->bom();
  }
  return boost::none;
}

void TextEditView::setBOM(const core::BOM& bom) {
  if (d_ptr->m_document) {
    d_ptr->m_document->setBOM(bom);
  }
}

void TextEditView::setPath(const QString& path) {
  if (path.isEmpty())
    return;

  d_ptr->m_document->setPath(path);
}

void TextEditView::find(const QString& text, int begin, int end, Document::FindFlags flags) {
  find(text, textCursor(), begin, end, flags);
}

void TextEditView::find(const QString& text,
                        int searchStartPos,
                        int begin,
                        int end,
                        Document::FindFlags flags) {
  qDebug("find: searchStartPos: %d, begin: %d, end: %d", searchStartPos, begin, end);
  QTextCursor cursor(document());
  cursor.setPosition(searchStartPos);
  find(text, cursor, begin, end, flags);
}

void TextEditView::find(const QString& text,
                        const QTextCursor& cursor,
                        int begin,
                        int end,
                        Document::FindFlags flags) {
  if (text.isEmpty())
    return;
  if (Document* doc = document()) {
    const QTextCursor& resultCursor = doc->find(text, cursor, begin, end, flags);
    if (!resultCursor.isNull()) {
      setTextCursor(resultCursor);
    } else {
      QTextCursor nextFindCursor(doc);
      if (flags.testFlag(Document::FindFlag::FindBackward)) {
        if (end < 0) {
          nextFindCursor.movePosition(QTextCursor::End);
        } else {
          nextFindCursor.setPosition(end);
        }
      } else {
        nextFindCursor.setPosition(begin);
      }
      const QTextCursor& cursor2 = doc->find(text, nextFindCursor, begin, end, flags);
      if (!cursor2.isNull()) {
        setTextCursor(cursor2);
      }
    }
  }
}

int TextEditView::lineNumberAreaWidth() {
  int digits = 3;
  int max = qMax(1, blockCount());
  while (max >= 1000) {
    max /= 10;
    ++digits;
  }

  int space = 10 + Config::singleton().fontMetrics().width(QLatin1Char('9')) * digits;

  return space;
}

void TextEditView::moveCursor(int mv, int n) {
  QTextCursor cur = textCursor();
  const int pos = cur.position();
  QTextBlock block = cur.block();
  const int blockPos = block.position();
  const QString blockText = block.text();
  bool moved = false;
  switch (mv) {
    case QTextCursor::Left: {
      n = qMin(n, pos - blockPos);
      break;
    }
    case QTextCursor::Right: {
      const QString text = block.text();
      if (text.isEmpty())
        return;  // new line or EOF only
      int endpos = blockPos + text.length();
      // If the cursor is block mode, don't allow it to move at EOL
      if (!isThinCursor()) {
        endpos -= 1;
      }
      if (pos >= endpos)
        return;
      n = qMin(n, endpos - pos);
      break;
    }
    case ViMoveOperation::FirstNonBlankChar:
      cur.setPosition(blockPos + firstNonBlankCharPos(blockText));
      moved = true;
      break;
    case ViMoveOperation::LastChar: {
      int ix = blockText.length();
      if (ix != 0)
        --ix;
      cur.setPosition(blockPos + ix);
      moved = true;
      break;
    }
    case ViMoveOperation::NextLine:
      cur.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, n);
      moveToFirstNonBlankChar(cur);
      moved = true;
      break;
    case ViMoveOperation::PrevLine:
      cur.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor, n);
      moveToFirstNonBlankChar(cur);
      moved = true;
      break;
    default:
      break;
  }

  if (!moved) {
    cur.movePosition(static_cast<QTextCursor::MoveOperation>(mv), QTextCursor::MoveAnchor, n);
  }

  setTextCursor(cur);
}

void TextEditView::performCompletion() {
  QTextCursor cursor = textCursor();
  cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
  const QString completionPrefix = cursor.selectedText();
  if (!completionPrefix.isEmpty() &&
      completionPrefix.at(completionPrefix.length() - 1).isLetter()) {
    d_ptr->performCompletion(completionPrefix);
  }
}

void TextEditView::resizeEvent(QResizeEvent* e) {
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  d_ptr->m_lineNumberArea->setGeometry(
      QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextEditView::lineNumberAreaPaintEvent(QPaintEvent* event) {
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

void TextEditView::paintEvent(QPaintEvent* e) {
  QPlainTextEdit::paintEvent(e);

  QPainter painter(viewport());

  // highlight search matched texts
  foreach (const Region& region, d_ptr->m_searchMatchedRegions) {
    QTextCursor beginCursor(document()->docHandle(), region.begin());
    QTextCursor endCursor(document()->docHandle(), region.end() - 1);
    int beginPos = beginCursor.positionInBlock();
    int endPos = endCursor.positionInBlock();
    QTextBlock block = beginCursor.block();
    QTextLine textLine = block.layout()->lineForTextPosition(beginPos);
    QRectF lineRect = textLine.naturalTextRect();
    lineRect.setLeft(textLine.cursorToX(beginPos));
    lineRect.setRight(textLine.cursorToX(endPos));
    lineRect = lineRect.translated(blockBoundingGeometry(block).topLeft() + contentOffset());
    painter.drawRoundedRect(lineRect, 3.0, 3.0);
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

void TextEditView::setFontPointSize(int sz) {
  QFont ft = font();
  ft.setPointSize(sz);
  setFont(ft);
}

void TextEditView::makeFontBigger(bool bigger) {
  int sz = font().pointSize();
  if (bigger) {
    ++sz;
  } else if (!--sz)
    return;
  setFontPointSize(sz);
}

int TextEditView::firstNonBlankCharPos(const QString& text) {
  int ix = 0;
  while (ix < text.length() && isTabOrSpace(text[ix])) {
    ++ix;
  }
  return ix;
}

inline bool TextEditView::isTabOrSpace(const QChar ch) {
  return ch == '\t' || ch == ' ';
}

void TextEditView::moveToFirstNonBlankChar(QTextCursor& cur) {
  QTextBlock block = cur.block();
  const int blockPos = block.position();
  const QString blockText = block.text();
  if (!blockText.isEmpty()) {
    cur.setPosition(blockPos + firstNonBlankCharPos(blockText));
  }
}

void TextEditView::setViewportMargins(int left, int top, int right, int bottom) {
  QPlainTextEdit::setViewportMargins(left, top, right, bottom);
}

void TextEditView::setTheme(core::Theme* theme) {
  Q_D(TextEditView);
  d->setTheme(theme);
}

void TextEditView::highlightSearchMatches(const QString& text,
                                          int begin,
                                          int end,
                                          Document::FindFlags flags) {
  d_ptr->m_searchMatchedRegions.clear();

  QTextCursor cursor(document());
  cursor.setPosition(begin);

  while (!cursor.isNull() && !cursor.atEnd()) {
    cursor = document()->find(text, cursor, begin, end, flags);
    if (!cursor.isNull()) {
      d_ptr->m_searchMatchedRegions.append(
          Region(cursor.selectionStart(), cursor.selectionEnd() + 1));
    }
  }
  update();
}

void TextEditView::clearSearchHighlight() {
  d_ptr->m_searchMatchedRegions.clear();
  update();
}

void TextEditView::replaceSelection(const QString& text, bool preserveCase) {
  QTextCursor cursor = textCursor();
  if (cursor.hasSelection()) {
    cursor.beginEditBlock();
    insertText(cursor, text, preserveCase);
    cursor.endEditBlock();
  }
}

void TextEditView::replaceAllSelection(const QString& findText,
                                       const QString& replaceText,
                                       int begin,
                                       int end,
                                       Document::FindFlags flags,
                                       bool preserveCase) {
  if (Document* doc = document()) {
    QTextCursor currentCursor = textCursor();
    currentCursor.beginEditBlock();

    QTextCursor cursor(doc);
    cursor.setPosition(begin);
    while (!cursor.isNull() && !cursor.atEnd()) {
      cursor = doc->find(findText, cursor, begin, end, flags);
      if (!cursor.isNull()) {
        Q_ASSERT(cursor.hasSelection());
        insertText(cursor, replaceText, preserveCase);
      }
    }

    currentCursor.endEditBlock();
    update();
    clearSearchHighlight();
  }
}

void TextEditView::insertNewLineWithIndent() {
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
  int tabWidth = Config::singleton().tabWidth();
  auto cursor = textCursor();
  TextEditViewLogic::indentCurrentLine(d_ptr->m_document.get(), cursor, prevLineString,
                                       prevPrevLineText, metadata, indentUsingSpaces, tabWidth);
}

void TextEditView::request(TextEditView* view,
                           const QString& method,
                           msgpack::rpc::msgid_t msgId,
                           const msgpack::object&) {
  if (method == "text") {
    PluginManager::singleton().sendResponse(view->toPlainText().toUtf8().constData(),
                                            msgpack::type::nil(), msgId);
  } else if (method == "scopeName") {
    QString scope = view->d_ptr->m_document->scopeName(view->textCursor().position());
    PluginManager::singleton().sendResponse(scope.toUtf8().constData(), msgpack::type::nil(),
                                            msgId);
  } else if (method == "scopeTree") {
    QString scopeTree = view->d_ptr->m_document->scopeTree();
    PluginManager::singleton().sendResponse(scopeTree.toUtf8().constData(), msgpack::type::nil(),
                                            msgId);
  } else {
    qWarning("%s is not supported", qPrintable(method));
    PluginManager::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
  }
}

void TextEditView::notify(TextEditView* view, const QString& method, const msgpack::object& obj) {
  int numArgs = obj.via.array.size;
  if (method == "save") {
    view->save();
  } else if (method == "saveAs") {
    view->saveAs();
  } else if (method == "undo") {
    view->undo();
  } else if (method == "redo") {
    view->redo();
  } else if (method == "cut") {
    view->cut();
  } else if (method == "copy") {
    view->copy();
  } else if (method == "paste") {
    view->paste();
  } else if (method == "selectAll") {
    view->selectAll();
  } else if (method == "complete") {
    view->performCompletion();
  } else if (method == "delete") {
    std::tuple<int, int> params;
    obj.convert(&params);
    int repeat = std::get<1>(params);
    qDebug("repeat: %d", repeat);
    view->doDelete(repeat);
  } else if (method == "moveCursor") {
    if (numArgs == 3) {
      std::tuple<int, std::string, int> params;
      obj.convert(&params);
      std::string operation = std::get<1>(params);
      int repeat = std::get<2>(params);
      qDebug("operation: %s", operation.c_str());
      qDebug("repeat: %d", repeat);
      view->moveCursor(toMoveOperation(std::move(operation)), repeat);
    } else {
      qWarning("invalid numArgs: %d", numArgs);
    }
  } else if (method == "setThinCursor") {
    if (numArgs == 2) {
      std::tuple<int, bool> params;
      obj.convert(&params);
      bool isThin = std::get<1>(params);
      view->setThinCursor(isThin);
    } else {
      qWarning("invalid numArgs: %d", numArgs);
    }
  } else if (method == "insertNewLine") {
    view->insertNewLineWithIndent();
  } else if (method == "indent") {
    auto cursor = view->textCursor();
    view->d_ptr->indentOneLevel(cursor);
  } else {
    qWarning("%s is not support", qPrintable(method));
  }
}

TextEditView* TextEditView::clone() {
  TextEditView* editView = new TextEditView(this);
  editView->setDocument(d_ptr->m_document);
  return editView;
}

void TextEditView::save() {
  if (DocumentManager::save(d_ptr->m_document.get())) {
    emit saved();
  }
}

void TextEditView::saveAs() {
  QString newFilePath = DocumentManager::saveAs(d_ptr->m_document.get());
  if (!newFilePath.isEmpty()) {
    setPath(newFilePath);
    emit saved();
  }
}

void TextEditView::doDelete(int n) {
  QTextCursor cur = textCursor();
  if (!cur.hasSelection()) {
    const int pos = cur.position();
    int dst;
    if (n > 0) {
      cur.movePosition(QTextCursor::EndOfBlock);
      const int endpos = cur.position();
      if (pos == endpos)
        return;
      dst = qMin(pos + n, endpos);
      cur.setPosition(pos);
    } else {
      const int blockPos = cur.block().position();
      if (pos == blockPos)
        return;
      dst = qMax(pos + n, blockPos);
    }

    cur.setPosition(dst, QTextCursor::KeepAnchor);
  }

  cur.deleteChar();
}

void TextEditView::doUndo(int n) {
  for (int i = 0; i < n; i++) {
    undo();
  }
}

void TextEditView::doRedo(int n) {
  for (int i = 0; i < n; i++) {
    redo();
  }
}

bool TextEditView::isThinCursor() {
  return !overwriteMode();
}

void TextEditView::setThinCursor(bool on) {
  setOverwriteMode(!on);
  update();
}

void TextEditView::wheelEvent(QWheelEvent* e) {
  Qt::KeyboardModifiers mod = e->modifiers();
  if ((mod & Qt::ControlModifier) != 0) {
    makeFontBigger(e->delta() > 0);
  } else {
    QPlainTextEdit::wheelEvent(e);
  }
}

void TextEditView::keyPressEvent(QKeyEvent* event) {
  if (d_ptr->m_completedAndSelected && d_ptr->handledCompletedAndSelected(event)) {
    return;
  }
  d_ptr->m_completedAndSelected = false;

  if (d_ptr->m_completer->popup()->isVisible()) {
    switch (event->key()) {
      case Qt::Key_Up:
      case Qt::Key_Down:
      case Qt::Key_Enter:
      case Qt::Key_Return:
      case Qt::Key_Escape:
        event->ignore();
        return;
      default:
        d_ptr->m_completer->popup()->hide();
        break;
    }
  }

  // todo: define this behavior in keymap.yml
  switch (event->key()) {
    case Qt::Key_Escape:
      API::hideActiveFindReplacePanel();
      clearSelection();
      break;
  }

  if (TextEditViewKeyHandler::singleton().dispatchKeyPressEvent(event)) {
    return;
  }

  QPlainTextEdit::keyPressEvent(event);
}

void TextEditView::mousePressEvent(QMouseEvent* event) {
  if (d_ptr->m_completedAndSelected) {
    d_ptr->m_completedAndSelected = false;
    QTextCursor cursor = textCursor();
    cursor.removeSelectedText();
    setTextCursor(cursor);
  }

  QPlainTextEdit::mousePressEvent(event);
}

void TextEditView::clearSelection() {
  QTextCursor cursor = textCursor();
  if (cursor.hasSelection()) {
    cursor.clearSelection();
    setTextCursor(cursor);
  }
}

#include "moc_TextEditView.cpp"
