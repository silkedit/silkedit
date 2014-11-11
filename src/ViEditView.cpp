#include <QtWidgets>

#include "vi.h"
#include "ViEditView.h"
#include "KeymapService.h"
#include "DefaultCursorDrawer.h"
#include "CommandService.h"
#include "commands/MoveCursorCommand.h"
#include "commands/DeleteCommand.h"
#include "commands/UndoCommand.h"
#include "commands/RedoCommand.h"
#include "commands/EvalAsRubyCommand.h"

ViEditView::ViEditView(QWidget* parent)
    : QPlainTextEdit(parent), m_cursorDrawer(new DefaultCursorDrawer) {
  // add commands
  std::unique_ptr<MoveCursorCommand> moveCursorCmd(new MoveCursorCommand(this));
  CommandService::singleton().addCommand(std::move(moveCursorCmd));

  std::unique_ptr<DeleteCommand> deleteCmd(new DeleteCommand(this));
  CommandService::singleton().addCommand(std::move(deleteCmd));

  std::unique_ptr<UndoCommand> undoCmd(new UndoCommand(this));
  CommandService::singleton().addCommand(std::move(undoCmd));

  std::unique_ptr<RedoCommand> redoCmd(new RedoCommand(this));
  CommandService::singleton().addCommand(std::move(redoCmd));

  std::unique_ptr<EvalAsRubyCommand> evalAsRubyCmd(new EvalAsRubyCommand(this));
  CommandService::singleton().addCommand(std::move(evalAsRubyCmd));

  m_lineNumberArea = new LineNumberArea(this);

  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));
  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateCursor()));

  updateLineNumberAreaWidth(0);
  highlightCurrentLine();
}

int ViEditView::lineNumberAreaWidth() {
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }

  int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

  return space;
}

void ViEditView::moveCursor(int mv, int n) {
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

void ViEditView::updateLineNumberAreaWidth(int /* newBlockCount */) {
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void ViEditView::updateLineNumberArea(const QRect& rect, int dy) {
  if (dy)
    m_lineNumberArea->scroll(0, dy);
  else
    m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

  if (rect.contains(viewport()->rect()))
    updateLineNumberAreaWidth(0);
}

void ViEditView::resizeEvent(QResizeEvent* e) {
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void ViEditView::highlightCurrentLine() {
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

void ViEditView::lineNumberAreaPaintEvent(QPaintEvent* event) {
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

void ViEditView::updateCursor() {
  m_cursorDrawer->updateCursor(*this);
}

void ViEditView::keyPressEvent(QKeyEvent* e) {
  bool isHandled = KeymapService::singleton().dispatch(static_cast<QKeyEvent*>(e));
  if (!isHandled) {
    QPlainTextEdit::keyPressEvent(e);
  }
}

void ViEditView::paintEvent(QPaintEvent* e) {
  drawCursor();
  setCursorWidth(0);
  QPlainTextEdit::paintEvent(e);
  setCursorWidth(m_cursorDrawer->cursorWidth());

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

void ViEditView::drawCursor() {
  if (!m_cursorDrawer) {
    qCritical() << "cursorDrawer is null!";
    return;
  }

  QPainter painter(viewport());
  auto tuple = m_cursorDrawer->draw(cursorRect());
  QRect& r = std::get<0>(tuple);
  QColor& color = std::get<1>(tuple);

  painter.fillRect(r, color);
}

void ViEditView::setFontPointSize(int sz) {
  QFont ft = font();
  ft.setPointSize(sz);
  setFont(ft);
}

void ViEditView::makeFontBigger(bool bigger) {
  int sz = font().pointSize();
  if (bigger) {
    ++sz;
  } else if (!--sz)
    return;
  setFontPointSize(sz);
}

int ViEditView::firstNonBlankCharPos(const QString& text) {
  int ix = 0;
  while (ix < text.length() && isTabOrSpace(text[ix])) {
    ++ix;
  }
  return ix;
}

inline bool ViEditView::isTabOrSpace(const QChar ch) {
  return ch == '\t' || ch == ' ';
}

void ViEditView::moveToFirstNonBlankChar(QTextCursor& cur) {
  QTextBlock block = cur.block();
  const int blockPos = block.position();
  const QString blockText = block.text();
  if (!blockText.isEmpty()) {
    cur.setPosition(blockPos + firstNonBlankCharPos(blockText));
  }
}

void ViEditView::doDelete(int n) {
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

void ViEditView::doUndo(int n) {
  for (int i = 0; i < n; i++) {
    undo();
  }
}

void ViEditView::doRedo(int n) {
  for (int i = 0; i < n; i++) {
    redo();
  }
}

void ViEditView::wheelEvent(QWheelEvent* e) {
  Qt::KeyboardModifiers mod = e->modifiers();
  if ((mod & Qt::ControlModifier) != 0) {
    makeFontBigger(e->delta() > 0);
  } else {
    QPlainTextEdit::wheelEvent(e);
  }
}
