#include <QtWidgets>

#include "vi.h"
#include "TextEditView.h"
#include "KeymapService.h"
#include "CommandService.h"
#include "OpenRecentItemService.h"
#include "DocumentService.h"

namespace {
const QString DEFAULT_SCOPE = "text.plain";
}

TextEditView::TextEditView(QWidget* parent) : STextEdit(parent) {
  m_lineNumberArea = new LineNumberArea(this);

  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));
  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

  connect(this,
          SIGNAL(destroying(const QString&)),
          &OpenRecentItemService::singleton(),
          SLOT(addOpenRecentItem(const QString&)));

  updateLineNumberAreaWidth(0);
  highlightCurrentLine();

  // Solarized Light
  setStyleSheet(
      "STextEdit{color: #657b83; background-color: #fdf6e3;"
      " selection-background-color: #93a1a1; selection-color: #eee8d5;}");

  QApplication::setCursorFlashTime(0);
  installEventFilter(&KeyHandler::singleton());
  setLanguage(DEFAULT_SCOPE);
}

TextEditView::~TextEditView() {
  emit destroying(m_document->path());
  qDebug("~TextEditView");
}

QString TextEditView::path() { return m_document ? m_document->path() : ""; }

void TextEditView::setDocument(std::shared_ptr<Document> document) {
  m_document = document;
  STextEdit::setDocument(document.get());
  updateLineNumberAreaWidth(blockCount());
}

Language* TextEditView::language() {
  if (m_document) {
    return m_document->language();
  }
  return nullptr;
}

void TextEditView::setLanguage(const QString& scopeName) {
  if (m_document) {
    m_document->setLanguage(scopeName);
  }
}

void TextEditView::setPath(const QString& path) {
  if (path.isEmpty())
    return;

  m_document->setPath(path);
  emit pathUpdated(path);
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
      const int endpos = blockPos + text.length() - 1;
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

void TextEditView::updateLineNumberAreaWidth(int /* newBlockCount */) {
  //  qDebug("updateLineNumberAreaWidth");
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TextEditView::updateLineNumberArea(const QRect& rect, int dy) {
  //  qDebug("updateLineNumberArea");
  if (dy)
    m_lineNumberArea->scroll(0, dy);
  else
    m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

  if (rect.contains(viewport()->rect()))
    updateLineNumberAreaWidth(0);
}

void TextEditView::resizeEvent(QResizeEvent* e) {
  STextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextEditView::highlightCurrentLine() {
  //  QList<QTextEdit::ExtraSelection> extraSelections;

  //  if (!isReadOnly()) {
  //    QTextEdit::ExtraSelection selection;

  //    QColor lineColor = QColor(Qt::yellow).lighter(160);

  //    selection.format.setBackground(lineColor);
  //    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
  //    selection.cursor = textCursor();
  //    selection.cursor.clearSelection();
  //    extraSelections.append(selection);
  //  }

  //  setExtraSelections(extraSelections);
}

void TextEditView::lineNumberAreaPaintEvent(QPaintEvent* event) {
  QPainter painter(m_lineNumberArea);
  painter.fillRect(event->rect(), Qt::lightGray);

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
  int bottom = top + (int)blockBoundingRect(block).height();

  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      QString number = QString::number(blockNumber + 1);
      painter.setPen(Qt::black);
      painter.drawText(
          0, top, m_lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
    }

    block = block.next();
    top = bottom;
    bottom = top + (int)blockBoundingRect(block).height();
    ++blockNumber;
  }
}

void TextEditView::paintEvent(QPaintEvent* e) {
  STextEdit::paintEvent(e);

  const int bottom = viewport()->rect().height();
  QPainter painter(viewport());
  painter.setPen(Qt::blue);
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
    painter.drawText(QPointF(r.left(), r.bottom()), "←");
    block = block.next();
  }
  cur.movePosition(QTextCursor::End);
  QRect r = cursorRect(cur);
  painter.drawText(QPointF(r.left(), r.bottom()), "[EOF]");
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

inline bool TextEditView::isTabOrSpace(const QChar ch) { return ch == '\t' || ch == ' '; }

void TextEditView::moveToFirstNonBlankChar(QTextCursor& cur) {
  QTextBlock block = cur.block();
  const int blockPos = block.position();
  const QString blockText = block.text();
  if (!blockText.isEmpty()) {
    cur.setPosition(blockPos + firstNonBlankCharPos(blockText));
  }
}

TextEditView* TextEditView::clone() {
  TextEditView* editView = new TextEditView(this);
  editView->setDocument(m_document);
  return editView;
}

void TextEditView::save() {
  DocumentService::singleton().save(m_document.get());
  emit saved();
}

void TextEditView::saveAs() {
  QString newFilePath = DocumentService::singleton().saveAs(m_document.get());
  if (!newFilePath.isEmpty()) {
    setPath(newFilePath);
  }
  emit saved();
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

void TextEditView::setThinCursor(bool on) {
  setOverwriteMode(!on);
  update();
}

void TextEditView::wheelEvent(QWheelEvent* e) {
  Qt::KeyboardModifiers mod = e->modifiers();
  if ((mod & Qt::ControlModifier) != 0) {
    makeFontBigger(e->delta() > 0);
  } else {
    STextEdit::wheelEvent(e);
  }
}
