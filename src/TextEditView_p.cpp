#include <QtWidgets>
#include <QStringBuilder>

#include "TextEditView_p.h"
#include "core/TextEditViewLogic.h"
#include "core/Session.h"
#include "core/Metadata.h"
#include "core/Theme.h"
#include "core/Document.h"

using core::Encoding;
using core::Metadata;
using core::Theme;
using core::ColorSettings;
using core::Regexp;
using core::Session;
using core::TextEditViewLogic;
using core::Document;

namespace {
bool caseInsensitiveLessThan(const QString& a, const QString& b) {
  return a.compare(b, Qt::CaseInsensitive) < 0;
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

void TextEditViewPrivate::setTabStopWidthFromSession() {
  QFontMetrics metrics(Session::singleton().font());
  q_ptr->setTabStopWidth(Session::singleton().tabWidth() * metrics.width(" "));
}

void TextEditViewPrivate::setupConnections(std::shared_ptr<core::Document> document) {
  Q_Q(TextEditView);

  // QObject::disconnect from old document
  QObject::disconnect(m_document.get(), &Document::pathUpdated, q, &TextEditView::pathUpdated);
  QObject::disconnect(m_document.get(), &Document::languageChanged, q,
                      &TextEditView::languageChanged);
  QObject::disconnect(m_document.get(), &Document::encodingChanged, q,
                      &TextEditView::encodingChanged);
  QObject::disconnect(m_document.get(), &Document::lineSeparatorChanged, q,
                      &TextEditView::lineSeparatorChanged);
  QObject::disconnect(m_document.get(), SIGNAL(contentsChanged), q,
                      SLOT(outdentCurrentLineIfNecessary));

  m_document = document;
  QObject::connect(m_document.get(), &Document::pathUpdated, q, &TextEditView::pathUpdated);
  QObject::connect(m_document.get(), &Document::languageChanged, q, &TextEditView::languageChanged);
  QObject::connect(m_document.get(), &Document::encodingChanged, q, &TextEditView::encodingChanged);
  QObject::connect(m_document.get(), &Document::lineSeparatorChanged, q,
                   &TextEditView::lineSeparatorChanged);
  QObject::connect(m_document.get(), SIGNAL(contentsChanged), q,
                   SLOT(outdentCurrentLineIfNecessary));
}

void TextEditViewPrivate::highlightCurrentLine() {
  if (q_ptr->textCursor().hasSelection()) {
    return;
  }

  Theme* theme = Session::singleton().theme();
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
  TextEditViewLogic::indentOneLevel(currentVisibleCursor, Session::singleton().indentUsingSpaces(),
                                    Session::singleton().tabWidth());
}

/**
 * @brief Outdent one level
 * @param currentVisibleCursor
 */
TextEditViewPrivate::TextEditViewPrivate(TextEditView* q_ptr)
    : q_ptr(q_ptr), m_document(nullptr), m_completedAndSelected(false) {
}

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
          prevLineString, currentVisibleCursor.atBlockEnd(), Session::singleton().tabWidth())) {
    TextEditViewLogic::outdent(m_document.get(), currentVisibleCursor,
                               Session::singleton().tabWidth());
  }
}
