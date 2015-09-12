#include <tuple>
#include <string>
#include <algorithm>
#include <QtWidgets>
#include <QStringBuilder>

#include "vi.h"
#include "TextEditView_p.h"
#include "TextEditViewLogic.h"
#include "KeymapManager.h"
#include "CommandManager.h"
#include "OpenRecentItemManager.h"
#include "DocumentManager.h"
#include "Session.h"
#include "API.h"
#include "PluginManager.h"
#include "core/ConfigManager.h"
#include "core/Metadata.h"
#include "core/LanguageParser.h"
#include "core/Theme.h"

using core::ConfigManager;
using core::Document;
using core::Encoding;
using core::Language;
using core::Region;
using core::Metadata;

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
  } else if (str == "start_of_block") {
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
}

class LineNumberArea : public QWidget {
 public:
  LineNumberArea(TextEditView* editor) : QWidget(editor) { m_codeEditor = editor; }

  QSize sizeHint() const override { return QSize(m_codeEditor->lineNumberAreaWidth(), 0); }

 protected:
  void paintEvent(QPaintEvent* event) override { m_codeEditor->lineNumberAreaPaintEvent(event); }

 private:
  TextEditView* m_codeEditor;
};

TextEditView::TextEditView(QWidget* parent)
    : QPlainTextEdit(parent), d(new TextEditViewPrivate(this)) {
  d->m_lineNumberArea = new LineNumberArea(this);

  connect(this, &TextEditView::blockCountChanged, d.get(),
          &TextEditViewPrivate::updateLineNumberAreaWidth);
  connect(this, &TextEditView::updateRequest, d.get(), &TextEditViewPrivate::updateLineNumberArea);
  connect(this, &TextEditView::cursorPositionChanged, d.get(),
          &TextEditViewPrivate::highlightCurrentLine);
  connect(this, &TextEditView::destroying, &OpenRecentItemManager::singleton(),
          &OpenRecentItemManager::addOpenRecentItem);
  connect(&Session::singleton(), &Session::themeChanged, d.get(), &TextEditViewPrivate::setTheme);
  connect(this, &TextEditView::saved, d.get(), &TextEditViewPrivate::clearDirtyMarker);
  connect(this, &TextEditView::copyAvailable, d.get(),
          &TextEditViewPrivate::toggleHighlightingCurrentLine);
  connect(&Session::singleton(), &Session::fontChanged, d.get(),
          &TextEditViewPrivate::setTabStopWidthFromSession);
  connect(&Session::singleton(), &Session::tabWidthChanged, d.get(),
          &TextEditViewPrivate::setTabStopWidthFromSession);

  d->updateLineNumberAreaWidth(0);

  QApplication::setCursorFlashTime(0);
  setLanguage(DEFAULT_SCOPE);
  d->setTheme(Session::singleton().theme());

  // setup for completion
  d->m_model.reset(new QStringListModel(this));
  d->m_completer.reset(new QCompleter(this));
  d->m_completer->setWidget(this);
  d->m_completer->setCompletionMode(QCompleter::PopupCompletion);
  d->m_completer->setModel(d->m_model.get());
  d->m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
  d->m_completer->setCaseSensitivity(Qt::CaseInsensitive);
  d->m_completer->setWrapAround(true);

  connect(d->m_completer.get(),
          static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::activated), d.get(),
          static_cast<void (TextEditViewPrivate::*)(const QString&)>(
              &TextEditViewPrivate::insertCompletion));
}

TextEditView::~TextEditView() {
  emit destroying(d->m_document->path());
  qDebug("~TextEditView");
}

QString TextEditView::path() {
  return d->m_document ? d->m_document->path() : "";
}

Document* TextEditView::document() {
  return d->m_document ? d->m_document.get() : nullptr;
}

void TextEditView::setDocument(std::shared_ptr<Document> document) {
  QPlainTextEdit::setDocument(document.get());

  d->setTabStopWidthFromSession();

  // Compare previous and current languages
  Language* prevLang = nullptr;
  Language* newLang = nullptr;
  if (d->m_document) {
    prevLang = d->m_document->language();
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
  if (d->m_document && document) {
    prevEnc = d->m_document->encoding();
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

  // disconnect from old document
  disconnect(d->m_document.get(), &Document::pathUpdated, this, &TextEditView::pathUpdated);
  disconnect(d->m_document.get(), &Document::languageChanged, d.get(),
             &TextEditViewPrivate::emitLanguageChanged);
  disconnect(d->m_document.get(), &Document::encodingChanged, d.get(),
             &TextEditViewPrivate::emitEncodingChanged);
  disconnect(d->m_document.get(), &Document::lineSeparatorChanged, d.get(),
             &TextEditViewPrivate::emitLineSeparatorChanged);
  disconnect(d->m_document.get(), &QTextDocument::contentsChanged, d.get(),
             &TextEditViewPrivate::outdentCurrentLineIfNecessary);

  d->m_document = document;
  d->updateLineNumberAreaWidth(blockCount());
  connect(d->m_document.get(), &Document::pathUpdated, this, &TextEditView::pathUpdated);
  connect(d->m_document.get(), &Document::languageChanged, d.get(),
          &TextEditViewPrivate::emitLanguageChanged);
  connect(d->m_document.get(), &Document::encodingChanged, d.get(),
          &TextEditViewPrivate::emitEncodingChanged);
  connect(d->m_document.get(), &Document::lineSeparatorChanged, d.get(),
          &TextEditViewPrivate::emitLineSeparatorChanged);
  connect(d->m_document.get(), &QTextDocument::contentsChanged, d.get(),
          &TextEditViewPrivate::outdentCurrentLineIfNecessary);
}

Language* TextEditView::language() {
  if (d->m_document) {
    return d->m_document->language();
  }
  return nullptr;
}

boost::optional<Encoding> TextEditView::encoding() {
  if (d->m_document) {
    return d->m_document->encoding();
  }
  return boost::none;
}

boost::optional<QString> TextEditView::lineSeparator() {
  if (d->m_document) {
    return d->m_document->lineSeparator();
  }
  return boost::none;
}

void TextEditView::setLineSeparator(const QString& lineSeparator) {
  if (d->m_document) {
    d->m_document->setLineSeparator(lineSeparator);
  }
}

void TextEditView::setLanguage(const QString& scopeName) {
  if (d->m_document) {
    d->m_document->setLanguage(scopeName);
  }
}

void TextEditView::setPath(const QString& path) {
  if (path.isEmpty())
    return;

  d->m_document->setPath(path);
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
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }

  int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

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
  cursor.select(QTextCursor::WordUnderCursor);
  const QString completionPrefix = cursor.selectedText();
  if (!completionPrefix.isEmpty() &&
      completionPrefix.at(completionPrefix.length() - 1).isLetter()) {
    d->performCompletion(completionPrefix);
  }
}

void TextEditView::resizeEvent(QResizeEvent* e) {
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  d->m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextEditView::lineNumberAreaPaintEvent(QPaintEvent* event) {
  QPainter painter(d->m_lineNumberArea);
  painter.fillRect(event->rect(), Qt::lightGray);

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
  int bottom = top + (int)blockBoundingRect(block).height();

  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      QString number = QString::number(blockNumber + 1);
      painter.setPen(Qt::black);
      painter.drawText(0, top, d->m_lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight,
                       number);
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
  foreach (const Region& region, d->m_searchMatchedRegions) {
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
    painter.setPen(Qt::red);
    painter.drawRoundedRect(lineRect, 0.0, 0.0);
  }

  // draw an end of line string
  const int bottom = viewport()->rect().height();
  if (ConfigManager::endOfLineColor().isValid()) {
    painter.setPen(ConfigManager::endOfLineColor());
  }
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
    if (!ConfigManager::endOfLineStr().isEmpty()) {
      painter.drawText(QPointF(r.left(), r.bottom()), ConfigManager::endOfLineStr());
    }
    block = block.next();
  }
  cur.movePosition(QTextCursor::End);
  QRect r = cursorRect(cur);

  // draw an end of file string
  if (!ConfigManager::endOfFileStr().isEmpty()) {
    if (ConfigManager::endOfFileColor().isValid()) {
      painter.setPen(ConfigManager::endOfFileColor());
    }
    painter.drawText(QPointF(r.left(), r.bottom()), ConfigManager::endOfFileStr());
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

void TextEditView::highlightSearchMatches(const QString& text,
                                          int begin,
                                          int end,
                                          Document::FindFlags flags) {
  d->m_searchMatchedRegions.clear();

  QTextCursor cursor(document());
  cursor.setPosition(begin);

  while (!cursor.isNull() && !cursor.atEnd()) {
    cursor = document()->find(text, cursor, begin, end, flags);
    if (!cursor.isNull()) {
      d->m_searchMatchedRegions.append(Region(cursor.selectionStart(), cursor.selectionEnd() + 1));
    }
  }
  update();
}

void TextEditView::clearSearchHighlight() {
  d->m_searchMatchedRegions.clear();
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
  auto metadata = Metadata::get(d->m_document->language()->scopeName);
  QString prevLineString;
  if (metadata) {
    prevLineString = d->prevLineText(1, metadata->unIndentedLinePattern());
  } else {
    prevLineString = d->prevLineText();
  }
  if (prevLineString.isEmpty()) {
    return;
  }

  boost::optional<QString> prevPrevLineText;
  if (metadata) {
    prevPrevLineText = d->prevLineText(2, metadata->unIndentedLinePattern());
  } else {
    prevPrevLineText = boost::none;
  }
  bool indentUsingSpaces = Session::singleton().indentUsingSpaces();
  int tabWidth = Session::singleton().tabWidth();
  auto cursor = textCursor();
  TextEditViewLogic::indentCurrentLine(d->m_document.get(), cursor, prevLineString,
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
    QString scope = view->d->m_document->scopeName(view->textCursor().position());
    PluginManager::singleton().sendResponse(scope.toUtf8().constData(), msgpack::type::nil(),
                                            msgId);
  } else if (method == "scopeTree") {
    QString scopeTree = view->d->m_document->scopeTree();
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
    view->d->indentOneLevel(cursor);
  } else {
    qWarning("%s is not support", qPrintable(method));
  }
}

TextEditView* TextEditView::clone() {
  TextEditView* editView = new TextEditView(this);
  editView->setDocument(d->m_document);
  return editView;
}

void TextEditView::save() {
  if (DocumentManager::save(d->m_document.get())) {
    emit saved();
  }
}

void TextEditView::saveAs() {
  QString newFilePath = DocumentManager::saveAs(d->m_document.get());
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
  if (d->m_completedAndSelected && d->handledCompletedAndSelected(event)) {
    return;
  }
  d->m_completedAndSelected = false;

  if (d->m_completer->popup()->isVisible()) {
    switch (event->key()) {
      case Qt::Key_Up:
      case Qt::Key_Down:
      case Qt::Key_Enter:
      case Qt::Key_Return:
      case Qt::Key_Escape:
        event->ignore();
        return;
      default:
        d->m_completer->popup()->hide();
        break;
    }
  }

  switch (event->key()) {
    case Qt::Key_Escape:
      API::hideActiveFindReplacePanel();
      break;
  }

  if (TextEditViewKeyHandler::singleton().dispatchKeyPressEvent(event)) {
    return;
  }

  QPlainTextEdit::keyPressEvent(event);
}

void TextEditView::mousePressEvent(QMouseEvent* event) {
  if (d->m_completedAndSelected) {
    d->m_completedAndSelected = false;
    QTextCursor cursor = textCursor();
    cursor.removeSelectedText();
    setTextCursor(cursor);
  }

  QPlainTextEdit::mousePressEvent(event);
}
