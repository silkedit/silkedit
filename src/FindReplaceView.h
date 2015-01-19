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

 protected:
  void showEvent(QShowEvent* event) override;

 private:
  LineEdit* m_lineEditForFind;
  QCheckBox* m_regexChk;
  QCheckBox* m_matchCaseChk;

  void findNext();
  void findPrev();
  void findText(const QString& text, Document::FindFlags flags = 0);
  void highlightMatches();
  void clearSearchHighlight();
  Document::FindFlags getFindFlags();
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

signals:
  void escapePressed();
  void shiftReturnPressed();
};
