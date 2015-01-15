#include <QtWidgets>
#include <QStringBuilder>

#include "vi.h"
#include "TextEditView.h"
#include "KeymapService.h"
#include "CommandService.h"
#include "OpenRecentItemService.h"
#include "DocumentService.h"
#include "Session.h"

namespace {
const QString DEFAULT_SCOPE = "text.plain";
}

TextEditView::TextEditView(QWidget* parent) : QPlainTextEdit(parent) {
  m_lineNumberArea = new LineNumberArea(this);

  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));
  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

  connect(this,
          SIGNAL(destroying(const QString&)),
          &OpenRecentItemService::singleton(),
          SLOT(addOpenRecentItem(const QString&)));
  connect(&Session::singleton(), SIGNAL(themeChanged(Theme*)), this, SLOT(changeTheme(Theme*)));

  updateLineNumberAreaWidth(0);

  QApplication::setCursorFlashTime(0);
  installEventFilter(&KeyHandler::singleton());
  setLanguage(DEFAULT_SCOPE);
  changeTheme(Session::singleton().theme());
}

TextEditView::~TextEditView() {
  emit destroying(m_document->path());
  qDebug("~TextEditView");
}

QString TextEditView::path() { return m_document ? m_document->path() : ""; }

void TextEditView::setDocument(std::shared_ptr<Document> document) {
  QPlainTextEdit::setDocument(document.get());
  Language* prevLang = nullptr;
  Language* newLang = nullptr;
  if (m_document) {
    prevLang = m_document->language();
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
  m_document = document;
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
    bool isSuccess = m_document->setLanguage(scopeName);
    if (isSuccess) {
      emit languageChanged(scopeName);
    }
  }
}

void TextEditView::setPath(const QString& path) {
  if (path.isEmpty())
    return;

  m_document->setPath(path);
  emit pathUpdated(path);
}

void TextEditView::find(const QString& text, QTextDocument::FindFlags flags) {
  qDebug("find: %s, %d", qPrintable(text), (int)(flags & QTextDocument::FindBackward));
  if (text.isEmpty())
    return;
  find(*Regexp::compile(text, Regexp::ASIS), flags);
}

void TextEditView::find(const Regexp& regexp, QTextDocument::FindFlags flags) {
  if (Document* doc = document()) {
    QTextCursor cursor = doc->find(regexp, textCursor(), flags);
    if (!cursor.isNull()) {
      setTextCursor(cursor);
    } else {
      QTextCursor nextFindCursor(doc);
      if (flags & QTextDocument::FindBackward) {
        nextFindCursor.movePosition(QTextCursor::End);
        Q_ASSERT(nextFindCursor.atEnd());
      } else {
        nextFindCursor.movePosition(QTextCursor::Start);
        Q_ASSERT(nextFindCursor.atStart());
      }
      cursor = doc->find(regexp, nextFindCursor, flags);
      if (!cursor.isNull()) {
        setTextCursor(cursor);
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

void TextEditView::changeTheme(Theme* theme) {
  qDebug("changeTheme");
  if (!theme) {
    qWarning("theme is null");
    return;
  }

  QString style;
  if (!theme->scopeSettings.isEmpty()) {
    ColorSettings* settings = theme->scopeSettings.first()->settings.get();
    if (settings->contains("foreground")) {
      style = style % QString("color: %1;").arg(settings->value("foreground").name());
      qDebug() << QString("color: %1;").arg(settings->value("foreground").name());
    }
    if (settings->contains("background")) {
      style = style % QString("background-color: %1;").arg(settings->value("background").name());
      qDebug() << QString("background-color: %1;").arg(settings->value("background").name());
    }
    if (settings->contains("selection")) {
      style = style %
              QString("selection-background-color: %1;").arg(settings->value("selection").name());
      qDebug() << QString("selection-background-color: %1;")
                      .arg(settings->value("selection").name());
    }
    if (settings->contains("selectionForeground")) {
      style = style %
              QString("selection-color: %1;").arg(settings->value("selectionForeground").name());
      qDebug() << QString("selection-color: %1;")
                      .arg(settings->value("selectionForeground").name());
    }

    setStyleSheet(QString("QPlainTextEdit{%1}").arg(style));
  }

  highlightCurrentLine();
}

void TextEditView::resizeEvent(QResizeEvent* e) {
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextEditView::highlightCurrentLine() {
  Theme* theme = Session::singleton().theme();
  if (theme && !theme->scopeSettings.isEmpty()) {
    ColorSettings* settings = theme->scopeSettings.first()->settings.get();
    if (settings->contains("lineHighlight")) {
      QList<QTextEdit::ExtraSelection> extraSelections;

      if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(settings->value("lineHighlight"));

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
      }

      setExtraSelections(extraSelections);
    } else {
      qDebug("lineHighlight not found");
    }
  } else {
    qDebug("theme is null or theme->scopeSettings is empty");
  }
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
  QPlainTextEdit::paintEvent(e);

  QPainter painter(viewport());

  // highlight search matched texts
  foreach(const Region & region, m_searchMatchedRegions) {
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

  // draw EOF
  const int bottom = viewport()->rect().height();
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

void TextEditView::highlightSearchMatches(const QString& text) {
  m_searchMatchedRegions.clear();

  QTextCursor cursor(document());

  while (!cursor.isNull() && !cursor.atEnd()) {
    cursor = document()->find(text, cursor);
    if (!cursor.isNull()) {
      m_searchMatchedRegions.append(Region(cursor.selectionStart(), cursor.selectionEnd() + 1));
    }
  }
  update();
}

void TextEditView::clearSearchHighlight() {
  m_searchMatchedRegions.clear();
  update();
}

TextEditView* TextEditView::clone() {
  TextEditView* editView = new TextEditView(this);
  editView->setDocument(m_document);
  return editView;
}

void TextEditView::save() {
  DocumentService::save(m_document.get());
  emit saved();
}

void TextEditView::saveAs() {
  QString newFilePath = DocumentService::saveAs(m_document.get());
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
    QPlainTextEdit::wheelEvent(e);
  }
}
