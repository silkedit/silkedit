#include <QtWidgets>

#include "vi.h"
#include "viEditView.h"
#include "viEngine.h"

ViEditView::ViEditView(QWidget *parent) : QPlainTextEdit(parent), m_mode(CMD) {
  m_lineNumberArea = new LineNumberArea(this);
  m_timer = new QElapsedTimer();
  m_timer->start();

  connect(this, SIGNAL(blockCountChanged(int)), this,
          SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(QRect, int)), this,
          SLOT(updateLineNumberArea(QRect, int)));
  connect(this, SIGNAL(cursorPositionChanged()), this,
          SLOT(highlightCurrentLine()));
  connect(this, SIGNAL(cursorPositionChanged()), this,
          SLOT(onCursorPositionChanged()));

  updateLineNumberAreaWidth(0);
  highlightCurrentLine();
}

ViEditView::~ViEditView() {}

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

void ViEditView::moveCursor(QTextCursor::MoveOperation mv, int n) {
  QTextCursor cur = textCursor();
  cur.movePosition(mv, QTextCursor::MoveAnchor, n);
  setTextCursor(cur);
}

void ViEditView::updateLineNumberAreaWidth(int /* newBlockCount */) {
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void ViEditView::updateLineNumberArea(const QRect &rect, int dy) {
  if (dy)
    m_lineNumberArea->scroll(0, dy);
  else
    m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(),
                             rect.height());

  if (rect.contains(viewport()->rect()))
    updateLineNumberAreaWidth(0);
}

void ViEditView::resizeEvent(QResizeEvent *e) {
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  m_lineNumberArea->setGeometry(
      QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

#if !USE_EVENT_FILTER
void ViEditView::keyPressEvent(QKeyEvent *event) {
  if (m_viEngine != 0 && m_viEngine->processKeyPressEvent(event)) {
    return;
  }
  QPlainTextEdit::keyPressEvent(event);
}
#endif

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

void ViEditView::lineNumberAreaPaintEvent(QPaintEvent *event) {
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
      painter.drawText(0, top, m_lineNumberArea->width(),
                       fontMetrics().height(), Qt::AlignRight, number);
    }

    block = block.next();
    top = bottom;
    bottom = top + (int)blockBoundingRect(block).height();
    ++blockNumber;
  }
}

void ViEditView::setMode(Mode mode) {
  m_tickCount = m_timer->elapsed();
  if (mode != m_mode) {
    m_mode = mode;
    emit modeChanged(mode);
    onCursorPositionChanged();
  }
}

void ViEditView::onCursorPositionChanged() {
  if (mode() == CMD) {
    QTextCursor cur = textCursor();
    cur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    QString text = cur.selectedText();
    QChar ch = text.isEmpty() ? QChar(' ') : text[0];
    int wd = fontMetrics().width(ch);
    if (!wd) {
      wd = fontMetrics().width(QChar(' '));
    }
    m_cursorWidth = wd;
  } else {
    m_cursorWidth = 1;
  }
}

void ViEditView::paintEvent(QPaintEvent *e) {
  const int blinkPeriod = 1200;
  qint64 tc = m_timer->elapsed() - m_tickCount;
  if (tc % blinkPeriod < blinkPeriod / 2) {
    drawCursor();
  }
  setCursorWidth(0);
  QPlainTextEdit::paintEvent(e);
  setCursorWidth(m_cursorWidth);

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
    if (r.top() >= bottom) break;
    painter.drawText(QPointF(r.left(), r.bottom()), "←");
    block = block.next();
  }
  cur.movePosition(QTextCursor::End);
  QRect r = cursorRect(cur);
  painter.drawText(QPointF(r.left(), r.bottom()), "[EOF]");
}

void ViEditView::drawCursor() {
  QPainter painter(viewport());
  QRect r = cursorRect();
  r.setWidth(m_cursorWidth);
  if (mode() == CMD) {
    r = QRect(r.left(), r.top() + r.height()/2, r.width(), r.height()/2);
  }
  painter.fillRect(r, Qt::red);
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
  } else if (!--sz) return;
  setFontPointSize(sz);
}

void ViEditView::wheelEvent(QWheelEvent *e) {
  Qt::KeyboardModifiers mod = e->modifiers();
  if ((mod & Qt::ControlModifier) != 0) {
    makeFontBigger(e->delta() > 0);
  } else {
    QPlainTextEdit::wheelEvent(e);
  }
}
