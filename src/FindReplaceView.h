#pragma once

#include <memory>
#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>

#include "core/macros.h"
#include "core/Document.h"
#include "core/HistoryModel.h"
#include "core/Region.h"

class LineEdit;
class TextEditView;

namespace Ui {
class FindReplaceView;
}
namespace core {
class Theme;
}

class FindReplaceView : public QWidget {
  Q_OBJECT
  DISABLE_COPY(FindReplaceView)

 public:
  explicit FindReplaceView(QWidget* parent);
  ~FindReplaceView();
  DEFAULT_MOVE(FindReplaceView)

  void highlightMatches();
  void setActiveView(QWidget* view);

public slots:
  void show();
  void hide();
  void findNext();
  void findPrevious();

 protected:
  void showEvent(QShowEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  bool focusNextPrevChild(bool next) override;

  // If you subclass from QWidget, you need to provide a paintEvent for your custom QWidget to
  // support stylesheet
  // http://doc.qt.io/qt-5/stylesheet-reference.html#list-of-stylable-widgets
  void paintEvent(QPaintEvent* event) override;

 private:
  std::unique_ptr<Ui::FindReplaceView> ui;

  int m_selectionStartPos;
  int m_selectionEndPos;
  int m_activeCursorPos;
  boost::optional<core::Region> m_selectedRegion;
  core::HistoryModel m_searchHistoryModel;
  core::HistoryModel m_replaceHistoryModel;
  QWidget* m_activeView;
  QMetaObject::Connection m_connectionForContentsChanged;
  QMetaObject::Connection m_connectionForCursorPositionChanged;

  void findFromActiveCursor();
  void findText(const QString& text, int searchStartPos, core::Document::FindFlags flags = 0);
  void findText(const QString& text, core::Document::FindFlags flags);
  void clearSearchHighlight();
  core::Document::FindFlags getFindFlags();
  void updateSelectionRegion();
  void updateActiveCursorPos();
  void selectFirstMatch();
  void replace();
  void replaceAll();

  void setTheme(core::Theme* theme);
};

Q_DECLARE_METATYPE(FindReplaceView*)
