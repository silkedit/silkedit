#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>

#include "core/macros.h"
#include "core/Document.h"
#include "core/HistoryModel.h"

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
  class CheckBox : public QCheckBox {
   public:
    CheckBox(const QString& text, FindReplaceView* parent);
  };

  LineEdit* m_lineEditForFind;
  LineEdit* m_lineEditForReplace;
  CheckBox* m_regexChk;
  CheckBox* m_matchCaseChk;
  CheckBox* m_wholeWordChk;
  CheckBox* m_inSelectionChk;
  CheckBox* m_preserveCaseChk;
  int m_selectionStartPos;
  int m_selectionEndPos;
  int m_activeCursorPos;
  core::HistoryModel m_searchHistoryModel;
  core::HistoryModel m_replaceHistoryModel;

  void findNext();
  void findPrev();
  void findFromActiveCursor();
  void findText(const QString& text, int searchStartPos = -1, core::Document::FindFlags flags = 0);
  void findText(const QString& text, core::Document::FindFlags flags);
  void highlightMatches();
  void clearSearchHighlight();
  core::Document::FindFlags getFindFlags();
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
