﻿#pragma once

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
#include "core/UniqueObject.h"

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
}

class TextEditView : public QPlainTextEdit,
                     public core::UniqueObject<TextEditView>,
                     public core::ICloneable<TextEditView> {
  Q_OBJECT
  Q_DECLARE_PRIVATE(TextEditView)
 public:
  explicit TextEditView(QWidget* parent);
  virtual ~TextEditView();

  QString path();
  core::Document* document();
  void setDocument(std::shared_ptr<core::Document> document);
  core::Language* language();
  void setLanguage(const QString& scopeName);
  boost::optional<core::Encoding> encoding();
  boost::optional<QString> lineSeparator();
  void setLineSeparator(const QString& lineSeparator);

  void lineNumberAreaPaintEvent(QPaintEvent* event);
  int lineNumberAreaWidth();
  void moveCursor(int mv, int = 1);
  void doDelete(int n);
  void doUndo(int n);
  void doRedo(int n);
  bool isThinCursor();
  void setThinCursor(bool on);
  TextEditView* clone() override;
  void save();
  void saveAs();
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
  void performCompletion();
  void insertNewLineWithIndent();
  void clearHighlightText();

signals:
  void destroying(const QString& path);
  void pathUpdated(const QString& path);
  void saved();
  void languageChanged(const QString& scope);
  // emitted when underlying document's encoding is changed.
  void encodingChanged(const core::Encoding& encoding);
  void lineSeparatorChanged(const QString& separator);

 protected:
  friend struct core::UniqueObject<TextEditView>;

  static void request(TextEditView* view,
                      const QString& method,
                      msgpack::rpc::msgid_t msgId,
                      const msgpack::object& obj);
  static void notify(TextEditView* view, const QString& method, const msgpack::object& obj);

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
  void setTheme(core::Theme *theme);
};
