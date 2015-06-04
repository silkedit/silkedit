#include <tuple>
#include <string>
#include <algorithm>
#include <QtWidgets>
#include <QStringBuilder>

#include "vi.h"
#include "TextEditView.h"
#include "KeymapManager.h"
#include "CommandManager.h"
#include "OpenRecentItemManager.h"
#include "DocumentManager.h"
#include "Session.h"
#include "API.h"
#include "PluginManager.h"
#include "ConfigManager.h"

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

TextEditView::TextEditView(QWidget* parent) : QPlainTextEdit(parent) {
  m_lineNumberArea = new LineNumberArea(this);

  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));
  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
  connect(this,
          SIGNAL(destroying(const QString&)),
          &OpenRecentItemManager::singleton(),
          SLOT(addOpenRecentItem(const QString&)));
  connect(&Session::singleton(), SIGNAL(themeChanged(Theme*)), this, SLOT(changeTheme(Theme*)));
  connect(this, &TextEditView::saved, this, &TextEditView::clearDirtyMarker);
  connect(this, &TextEditView::copyAvailable, this, &TextEditView::toggleHighlightingCurrentLine);

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

QString TextEditView::path() {
  return m_document ? m_document->path() : "";
}

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
  connect(m_document.get(), &Document::pathUpdated, this, &TextEditView::pathUpdated);
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

    setStyleSheet(QString("QPlainTextEdit{%1}").arg(style));
  }

  highlightCurrentLine();
}

void TextEditView::clearDirtyMarker() {
  document()->setModified(false);
}

void TextEditView::clearHighlightingCurrentLine() {
  setExtraSelections(QList<QTextEdit::ExtraSelection>());
}

void TextEditView::toggleHighlightingCurrentLine(bool hasSelection) {
  if (hasSelection) {
    clearHighlightingCurrentLine();
  } else {
    highlightCurrentLine();
  }
}

void TextEditView::resizeEvent(QResizeEvent* e) {
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextEditView::highlightCurrentLine() {
  if (textCursor().hasSelection()) {
    return;
  }

  Theme* theme = Session::singleton().theme();
  if (theme && !theme->scopeSettings.isEmpty()) {
    ColorSettings* settings = theme->scopeSettings.first()->colorSettings.get();
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
  foreach (const Region& region, m_searchMatchedRegions) {
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

void TextEditView::highlightSearchMatches(const QString& text,
                                          int begin,
                                          int end,
                                          Document::FindFlags flags) {
  m_searchMatchedRegions.clear();

  QTextCursor cursor(document());
  cursor.setPosition(begin);

  while (!cursor.isNull() && !cursor.atEnd()) {
    cursor = document()->find(text, cursor, begin, end, flags);
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

void TextEditView::request(TextEditView* view,
                           const QString& method,
                           msgpack::rpc::msgid_t msgId,
                           const msgpack::object&) {
  if (method == "text") {
    PluginManager::singleton().sendResponse(
        view->toPlainText().toUtf8().constData(), msgpack::type::nil(), msgId);
  } else if (method == "scopeName") {
    QString scope = view->m_document->scopeName(view->textCursor().position());
    PluginManager::singleton().sendResponse(
        scope.toUtf8().constData(), msgpack::type::nil(), msgId);
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
  } else {
    qWarning("%s is not support", qPrintable(method));
  }
}

TextEditView* TextEditView::clone() {
  TextEditView* editView = new TextEditView(this);
  editView->setDocument(m_document);
  return editView;
}

void TextEditView::save() {
  if (DocumentManager::save(m_document.get())) {
    emit saved();
  }
}

void TextEditView::saveAs() {
  QString newFilePath = DocumentManager::saveAs(m_document.get());
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
  switch (event->key()) {
    case Qt::Key_Escape:
      API::hideActiveFindReplacePanel();
      break;
  }
  QPlainTextEdit::keyPressEvent(event);
}
