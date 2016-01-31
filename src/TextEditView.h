#pragma once

#include <functional>
#include <memory>
#include <boost/optional.hpp>
#include <QObject>
#include <QPlainTextEdit>
#include <QHash>
#include <QMutex>
#include <QStringListModel>
#include <QCompleter>

#include "core/macros.h"
#include "core/ICloneable.h"
#include "core/Document.h"
#include "core/BOM.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QElapsedTimer;
class LineNumberArea;
class TextEditViewPrivate;
namespace core {
struct Language;
class Encoding;
class Theme;
class BOM;
}

class TextEditView : public QPlainTextEdit,
                     public core::ICloneable<TextEditView> {
  Q_OBJECT
  Q_DECLARE_PRIVATE(TextEditView)
 public:
  explicit TextEditView(QWidget* parent);
  virtual ~TextEditView();

  core::Document* document();
  void setDocument(std::shared_ptr<core::Document> document);
  core::Language* language();
  void setLanguage(const QString& scopeName);
  boost::optional<core::Encoding> encoding();
  boost::optional<QString> lineSeparator();
  void setLineSeparator(const QString& lineSeparator);
  boost::optional<core::BOM> BOM();
  void setBOM(const core::BOM& bom);

  void lineNumberAreaPaintEvent(QPaintEvent* event);
  int lineNumberAreaWidth();
  TextEditView* clone() override;
  void setPath(const QString& path);
  void find(const QString& text, int begin = 0, int end = -1, core::Document::FindFlags flags = 0);
  void find(const QString& text,
            int searchStartPos = -1,
            int begin = 0,
            int end = -1,
            core::Document::FindFlags flags = 0);
  void find(const QString& text,
            const QTextCursor& cursor,
            int begin = 0,
            int end = -1,
            core::Document::FindFlags flags = 0);
  void highlightSearchMatches(const QString& text,
                              int begin,
                              int end,
                              core::Document::FindFlags flags = 0);
  void clearSearchHighlight();
  void replaceSelection(const QString& text, bool preserveCase = false);
  void replaceAllSelection(const QString& findText,
                           const QString& replaceText,
                           int begin,
                           int end,
                           core::Document::FindFlags flags = 0,
                           bool preserveCase = false);
  void clearSelection();

 public slots:
  QString scopeName();
  QString scopeTree();
  void undo();
  void redo();
  void cut();
  void copy();
  void paste();
  void selectAll();
  void indent();
  QString text();
  void performCompletion();
  void insertNewLine();
  void save();
  void saveAs();
  void moveCursor(const QString& op, int repeat = 1);
  void deleteChar(int repeat = 1);
  bool isThinCursor();
  void setThinCursor(bool on);
  QString path();

 signals:
  void pathUpdated(const QString& path);
  void saved();
  void languageChanged(const QString& scope);
  // emitted when underlying document's encoding is changed.
  void encodingChanged(const core::Encoding& encoding);
  void lineSeparatorChanged(const QString& separator);
  void bomChanged(const core::BOM& bom);

  // private signals
  void destroying(const QString& path, QPrivateSignal);

 protected:
  void resizeEvent(QResizeEvent* event) override;
  void paintEvent(QPaintEvent* e) override;
  void wheelEvent(QWheelEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void mousePressEvent(QMouseEvent* e) override;
  void setFontPointSize(int sz);
  void makeFontBigger(bool bigger);
  int firstNonBlankCharPos(const QString& text);
  bool isTabOrSpace(const QChar ch);
  void moveToFirstNonBlankChar(QTextCursor& cur);

 private:
  TextEditViewPrivate* d_ptr;
  void setViewportMargins(int left, int top, int right, int bottom);

  Q_PRIVATE_SLOT(d_func(), void outdentCurrentLineIfNecessary())
  Q_PRIVATE_SLOT(d_func(), void insertCompletion(const QString& completion))
  Q_PRIVATE_SLOT(d_func(), void insertCompletion(const QString& completion, bool singleWord))
  Q_PRIVATE_SLOT(d_func(), void updateLineNumberAreaWidth(int newBlockCount))
  Q_PRIVATE_SLOT(d_func(), void updateLineNumberArea(const QRect&, int))
  Q_PRIVATE_SLOT(d_func(), void highlightCurrentLine())
  Q_PRIVATE_SLOT(d_func(), void clearDirtyMarker())
  Q_PRIVATE_SLOT(d_func(), void toggleHighlightingCurrentLine(bool hasSelection))
  Q_PRIVATE_SLOT(d_func(), void setTabStopWidthFromSession())

 private slots:
  void setTheme(core::Theme* theme);
};

Q_DECLARE_METATYPE(TextEditView*)
