#pragma once

#include <QWidget>
#include <QLineEdit>

#include "macros.h"
#include "Document.h"

class LineEdit;
class QCheckBox;

class FindReplaceView : public QWidget {
  Q_OBJECT
  DISABLE_COPY(FindReplaceView)

 public:
  explicit FindReplaceView(QWidget* parent);
  ~FindReplaceView() = default;
  DEFAULT_MOVE(FindReplaceView)

  void show();
  void hide();

 protected:
  void showEvent(QShowEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

 private:
  LineEdit* m_lineEditForFind;
  LineEdit* m_lineEditForReplace;
  QCheckBox* m_regexChk;
  QCheckBox* m_matchCaseChk;
  QCheckBox* m_wholeWordChk;
  QCheckBox* m_inSelectionChk;
  QCheckBox* m_preserveCaseChk;
  int m_selectionStartPos;
  int m_selectionEndPos;
  int m_activeCursorPos;

  void findNext();
  void findPrev();
  void findText(const QString& text, int searchStartPos = -1, Document::FindFlags flags = 0);
  void findText(const QString& text, Document::FindFlags flags);
  void highlightMatches();
  void clearSearchHighlight();
  Document::FindFlags getFindFlags();
  void updateSelectionRegion();
  void updateActiveCursorPos();
  void selectFirstMatch();
  void replace();
  void replaceAll();
};

class LineEdit : public QLineEdit {
  Q_OBJECT
  DISABLE_COPY(LineEdit)
 public:
  explicit LineEdit(QWidget* parent);
  ~LineEdit() = default;
  DEFAULT_MOVE(LineEdit)

 protected:
  void keyPressEvent(QKeyEvent* event) override;
  void focusInEvent(QFocusEvent* ev) override;

signals:
  void shiftReturnPressed();
  void focusIn();
};
