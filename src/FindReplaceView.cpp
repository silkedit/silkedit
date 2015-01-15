#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QKeySequence>
#include <QSizePolicy>

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
    : QWidget(parent), m_lineEditForFind(new LineEdit(this)) {
  QGridLayout* layout = new QGridLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  // QTBUG-14643: setSpacing(0) causes QCheckBox to overlap with another widgets.
  // https://bugreports.qt.io/browse/QTBUG-14643?jql=text%20~%20%22QGridLayout%20QCheckBox%22
  //  layout->setSpacing(0);

  LineEdit* lineEditForReplace = new LineEdit(this);
  m_lineEditForFind->setFixedWidth(lineEditWidth);
  lineEditForReplace->setFixedWidth(lineEditWidth);
  connect(m_lineEditForFind, &QLineEdit::returnPressed, this, &FindReplaceView::findNext);
  connect(m_lineEditForFind, &LineEdit::escapePressed, this, &FindReplaceView::hide);
  layout->addWidget(m_lineEditForFind, 0, 0);
  layout->addWidget(lineEditForReplace, 1, 0);

  QPushButton* findButton = new QPushButton(tr("Find"));
  QPushButton* replaceButton = new QPushButton(tr("Replace"));
  layout->addWidget(findButton, 0, 1);
  layout->addWidget(replaceButton, 1, 1);

  QPushButton* findAllButton = new QPushButton(tr("Find All"));
  QPushButton* replaceAllButton = new QPushButton(tr("Replace All"));
  layout->addWidget(findAllButton, 0, 2);
  layout->addWidget(replaceAllButton, 1, 2);

  QPushButton* prevButton = new QPushButton("▲");
  QPushButton* nextButton = new QPushButton("▼");
  layout->addWidget(prevButton, 0, 3);
  layout->addWidget(nextButton, 1, 3);

  QCheckBox* matchCaseChk = new QCheckBox(tr(MATCH_CASE_TEXT));
  QCheckBox* regexChk = new QCheckBox(tr(REGEX_TEXT));
  matchCaseChk->setToolTip(QKeySequence::mnemonic(MATCH_CASE_TEXT).toString());
  regexChk->setToolTip(QKeySequence::mnemonic(REGEX_TEXT).toString());
  layout->addWidget(matchCaseChk, 0, 4);
  layout->addWidget(regexChk, 1, 4);

  QCheckBox* wholeWordChk = new QCheckBox(tr(WHOLE_WORD_TEXT));
  QCheckBox* preserveCaseChk = new QCheckBox(tr(PRESERVE_CASE_TEXT));
  wholeWordChk->setToolTip(QKeySequence::mnemonic(WHOLE_WORD_TEXT).toString());
  preserveCaseChk->setToolTip(QKeySequence::mnemonic(PRESERVE_CASE_TEXT).toString());
  layout->addWidget(wholeWordChk, 1, 5);
  layout->addWidget(preserveCaseChk, 0, 5);

  QCheckBox* inSelectionChk = new QCheckBox(tr(IN_SELECTION_TEXT));
  inSelectionChk->setToolTip(QKeySequence::mnemonic(IN_SELECTION_TEXT).toString());
  layout->addWidget(inSelectionChk, 1, 6);

  layout->setColumnStretch(7, 1);

  setLayout(layout);
}

void FindReplaceView::showEvent(QShowEvent*) {
  m_lineEditForFind->setFocus();
  m_lineEditForFind->selectAll();
}

void FindReplaceView::findNext() {
  if (QLineEdit* findLineEdit = qobject_cast<QLineEdit*>(QObject::sender())) {
    findNextText(findLineEdit->text());
  }
}

void FindReplaceView::findNextText(const QString& text) {
  qDebug("findNextText: %s", qPrintable(text));
  if (text.isEmpty())
    return;

  if (TextEditView* editView = API::activeEditView()) {
    if (QTextDocument* doc = editView->document()) {
      QTextCursor cursor = doc->find(text, editView->textCursor());
      if (!cursor.isNull()) {
        editView->setTextCursor(cursor);
      } else {
        cursor = doc->find(text, 0);
        if (!cursor.isNull()) {
          editView->setTextCursor(cursor);
        }
      }
    }
  }
}

LineEdit::LineEdit(QWidget* parent) : QLineEdit(parent) {
  setClearButtonEnabled(true);
}

void LineEdit::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape) {
    emit escapePressed();
  }
  QLineEdit::keyPressEvent(event);
}
