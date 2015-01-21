#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QKeySequence>
#include <QSizePolicy>
#include <QCompleter>
#include <QCheckBox>

#include "FindReplaceView.h"
#include "API.h"
#include "TextEditView.h"

namespace {
constexpr auto MATCH_CASE_TEXT = "&Match Case";
constexpr auto REGEX_TEXT = "&Regex";
constexpr auto WHOLE_WORD_TEXT = "&Whole Word";
constexpr auto PRESERVE_CASE_TEXT = "&Preserve Case";
constexpr auto IN_SELECTION_TEXT = "&In Selection";
constexpr int lineEditWidth = 500;
}

FindReplaceView::FindReplaceView(QWidget* parent)
    : QWidget(parent),
      m_lineEditForFind(new LineEdit(this)),
      m_lineEditForReplace(new LineEdit(this)),
      m_regexChk(new CheckBox(tr(REGEX_TEXT), this)),
      m_matchCaseChk(new CheckBox(tr(MATCH_CASE_TEXT), this)),
      m_wholeWordChk(new CheckBox(tr(WHOLE_WORD_TEXT), this)),
      m_inSelectionChk(new CheckBox(tr(IN_SELECTION_TEXT), this)),
      m_preserveCaseChk(new CheckBox(tr(PRESERVE_CASE_TEXT), this)) {
  QGridLayout* layout = new QGridLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  // QTBUG-14643: setSpacing(0) causes QCheckBox to overlap with another widgets.
  // https://bugreports.qt.io/browse/QTBUG-14643?jql=text%20~%20%22QGridLayout%20QCheckBox%22
  //  layout->setSpacing(0);

  m_lineEditForFind->setFixedWidth(lineEditWidth);
  m_lineEditForReplace->setFixedWidth(lineEditWidth);
  QCompleter* searchCompleter = new QCompleter(this);
  searchCompleter->setModel(&m_searchHistoryModel);
  m_lineEditForFind->setCompleter(searchCompleter);
  QCompleter* replaceCompleter = new QCompleter(this);
  replaceCompleter->setModel(&m_replaceHistoryModel);
  m_lineEditForReplace->setCompleter(replaceCompleter);

  connect(m_lineEditForFind, &LineEdit::returnPressed, this, &FindReplaceView::findNext);
  connect(m_lineEditForFind, &LineEdit::shiftReturnPressed, this, &FindReplaceView::findPrev);
  connect(m_lineEditForFind, &LineEdit::textChanged, this, &FindReplaceView::highlightMatches);
  connect(m_lineEditForFind, &LineEdit::focusIn, this, &FindReplaceView::updateSelectionRegion);
  connect(m_lineEditForFind, &LineEdit::focusIn, this, &FindReplaceView::updateActiveCursorPos);
  connect(m_lineEditForFind, &LineEdit::focusIn, this, &FindReplaceView::highlightMatches);

  layout->addWidget(m_lineEditForFind, 0, 0);
  layout->addWidget(m_lineEditForReplace, 1, 0);

  QPushButton* replaceButton = new QPushButton(tr("Replace"));
  connect(replaceButton, &QPushButton::pressed, this, &FindReplaceView::replace);
  layout->addWidget(replaceButton, 1, 1);

  QPushButton* replaceAllButton = new QPushButton(tr("Replace All"));
  connect(replaceAllButton, &QPushButton::pressed, this, &FindReplaceView::replaceAll);
  layout->addWidget(replaceAllButton, 1, 2);

  QPushButton* prevButton = new QPushButton("▲");
  QPushButton* nextButton = new QPushButton("▼");
  connect(prevButton, &QPushButton::pressed, this, &FindReplaceView::findPrev);
  connect(nextButton, &QPushButton::pressed, this, &FindReplaceView::findNext);
  layout->addWidget(prevButton, 0, 3);
  layout->addWidget(nextButton, 1, 3);

  m_matchCaseChk->setToolTip(QKeySequence::mnemonic(MATCH_CASE_TEXT).toString());
  m_regexChk->setToolTip(QKeySequence::mnemonic(REGEX_TEXT).toString());
  layout->addWidget(m_matchCaseChk, 0, 4);
  layout->addWidget(m_regexChk, 1, 4);

  m_wholeWordChk->setToolTip(QKeySequence::mnemonic(WHOLE_WORD_TEXT).toString());
  m_preserveCaseChk->setToolTip(QKeySequence::mnemonic(PRESERVE_CASE_TEXT).toString());
  layout->addWidget(m_wholeWordChk, 1, 5);
  layout->addWidget(m_preserveCaseChk, 0, 5);

  m_inSelectionChk->setToolTip(QKeySequence::mnemonic(IN_SELECTION_TEXT).toString());
  layout->addWidget(m_inSelectionChk, 1, 6);

  layout->setColumnStretch(7, 1);

  setLayout(layout);
}

void FindReplaceView::show() {
  if (isVisible()) {
    showEvent(nullptr);
  }
  QWidget::show();
}

void FindReplaceView::showEvent(QShowEvent*) {
  m_lineEditForFind->setFocus();
  m_lineEditForFind->selectAll();
}

void FindReplaceView::keyPressEvent(QKeyEvent* event) {
  switch (event->key()) {
    case Qt::Key_Escape:
      hide();
      break;
  }
  QWidget::keyPressEvent(event);
}

void FindReplaceView::findNext() {
  Q_ASSERT(m_lineEditForFind);
  findText(m_lineEditForFind->text());
  m_searchHistoryModel.prepend(m_lineEditForFind->text());
}

void FindReplaceView::findPrev() {
  Q_ASSERT(m_lineEditForFind);
  findText(m_lineEditForFind->text(), Document::FindFlag::FindBackward);
  m_searchHistoryModel.prepend(m_lineEditForFind->text());
}

void FindReplaceView::findFromActiveCursor()
{
  findText(m_lineEditForFind->text(), m_activeCursorPos);
  m_searchHistoryModel.prepend(m_lineEditForFind->text());
}

void FindReplaceView::findText(const QString& text,
                               int searchStartPos,
                               Document::FindFlags otherFlags) {
  if (TextEditView* editView = API::activeEditView()) {
    Document::FindFlags flags = getFindFlags();
    flags |= otherFlags;
    int begin = 0, end = -1;
    if (flags.testFlag(Document::FindFlag::FindInSelection)) {
      qDebug("InSelection is on. begin: %d, end: %d", m_selectionStartPos, m_selectionEndPos);
      begin = m_selectionStartPos;
      end = m_selectionEndPos;
    }
    if (searchStartPos >= 0) {
      editView->find(text, searchStartPos, begin, end, flags);
    } else {
      editView->find(text, begin, end, flags);
    }
  }
}

void FindReplaceView::findText(const QString& text, Document::FindFlags flags) {
  findText(text, -1, flags);
}

void FindReplaceView::highlightMatches() {
  if (TextEditView* editView = API::activeEditView()) {
    int begin = 0, end = -1;
    if (m_inSelectionChk->isChecked()) {
      begin = m_selectionStartPos;
      end = m_selectionEndPos;
    }
    editView->highlightSearchMatches(m_lineEditForFind->text(), begin, end, getFindFlags());
    selectFirstMatch();
  }
}

void FindReplaceView::clearSearchHighlight() {
  if (TextEditView* editView = API::activeEditView()) {
    editView->clearSearchHighlight();
  }
}

Document::FindFlags FindReplaceView::getFindFlags() {
  Document::FindFlags flags;
  if (m_regexChk->isChecked()) {
    flags |= Document::FindFlag::FindRegex;
  }
  if (m_matchCaseChk->isChecked()) {
    flags |= Document::FindFlag::FindCaseSensitively;
  }
  if (m_wholeWordChk->isChecked()) {
    flags |= Document::FindFlag::FindWholeWords;
  }
  if (m_inSelectionChk->isChecked()) {
    flags |= Document::FindFlag::FindInSelection;
  }
  return flags;
}

void FindReplaceView::updateSelectionRegion() {
  if (TextEditView* editView = API::activeEditView()) {
    QTextCursor cursor = editView->textCursor();
    if (cursor.hasSelection()) {
      m_selectionStartPos = cursor.selectionStart();
      m_selectionEndPos = cursor.selectionEnd();
    } else {
      m_selectionStartPos = 0;
      m_selectionEndPos = -1;
    }
  }
}

void FindReplaceView::updateActiveCursorPos() {
  if (TextEditView* editView = API::activeEditView()) {
    m_activeCursorPos = editView->textCursor().selectionStart();
  }
}

void FindReplaceView::selectFirstMatch() {
  Q_ASSERT(m_lineEditForFind);
  findText(m_lineEditForFind->text(), m_activeCursorPos);
}

void FindReplaceView::replace() {
  Q_ASSERT(m_lineEditForReplace);
  if (TextEditView* editView = API::activeEditView()) {
    editView->replaceSelection(m_lineEditForReplace->text(), m_preserveCaseChk->isChecked());
    highlightMatches();
    m_replaceHistoryModel.prepend(m_lineEditForReplace->text());
  }
}

void FindReplaceView::replaceAll() {
  if (TextEditView* editView = API::activeEditView()) {
    int begin = 0, end = -1;
    if (m_inSelectionChk->isChecked()) {
      begin = m_selectionStartPos;
      end = m_selectionEndPos;
    }
    editView->replaceAllSelection(m_lineEditForFind->text(),
                                  m_lineEditForReplace->text(),
                                  begin,
                                  end,
                                  getFindFlags(),
                                  m_preserveCaseChk->isChecked());
    m_replaceHistoryModel.prepend(m_lineEditForReplace->text());
  }
}

void FindReplaceView::hide() {
  clearSearchHighlight();
  QWidget::hide();
}

LineEdit::LineEdit(QWidget* parent) : QLineEdit(parent) { setClearButtonEnabled(true); }

void LineEdit::keyPressEvent(QKeyEvent* event) {
  switch (event->key()) {
    case Qt::Key_Return:
      if (event->modifiers() & Qt::ShiftModifier) {
        emit shiftReturnPressed();
        return;
      }
  }

  QLineEdit::keyPressEvent(event);
}

void LineEdit::focusInEvent(QFocusEvent* ev) {
  emit focusIn();
  QLineEdit::focusInEvent(ev);
}

FindReplaceView::CheckBox::CheckBox(const QString &text, FindReplaceView *parent): QCheckBox(text, parent)
{
  connect(this, &QCheckBox::stateChanged, parent, &FindReplaceView::highlightMatches);

}
