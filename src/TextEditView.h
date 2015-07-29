#pragma once

#include <functional>
#include <memory>
#include <QObject>
#include <QPlainTextEdit>
#include <QHash>
#include <QMutex>
#include <QStringListModel>
#include <QCompleter>

#include "macros.h"
#include "ICloneable.h"
#include "Document.h"
#include "UniqueObject.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QElapsedTimer;

class LineNumberArea;
class TextEditViewPrivate;

class TextEditView : public QPlainTextEdit,
                     public UniqueObject<TextEditView>,
                     public ICloneable<TextEditView> {
  Q_OBJECT

 public:
  explicit TextEditView(QWidget* parent);
  virtual ~TextEditView();

  QString path();
  inline Document* document();
  void setDocument(std::shared_ptr<Document> document);
  Language* language();
  void setLanguage(const QString& scopeName);

  void lineNumberAreaPaintEvent(QPaintEvent* event);
  int lineNumberAreaWidth();
  void moveCursor(int mv, int = 1);
  void doDelete(int n);
  void doUndo(int n);
  void doRedo(int n);
  void setThinCursor(bool on);
  TextEditView* clone() override;
  void save();
  void saveAs();
  void setPath(const QString& path);
  void find(const QString& text, int begin = 0, int end = -1, Document::FindFlags flags = 0);
  void find(const QString& text,
            int searchStartPos = -1,
            int begin = 0,
            int end = -1,
            Document::FindFlags flags = 0);
  void find(const QString& text,
            const QTextCursor& cursor,
            int begin = 0,
            int end = -1,
            Document::FindFlags flags = 0);
  void highlightSearchMatches(const QString& text,
                              int begin,
                              int end,
                              Document::FindFlags flags = 0);
  void clearSearchHighlight();
  void replaceSelection(const QString& text, bool preserveCase = false);
  void replaceAllSelection(const QString& findText,
                           const QString& replaceText,
                           int begin,
                           int end,
                           Document::FindFlags flags = 0,
                           bool preserveCase = false);
  void performCompletion();
  void insertNewLineWithIndent();

signals:
  void destroying(const QString& path);
  void pathUpdated(const QString& path);
  void saved();
  void languageChanged(const QString& scope);

 protected:
  friend struct UniqueObject<TextEditView>;

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
  std::unique_ptr<TextEditViewPrivate> d;
  void setViewportMargins(int left, int top, int right, int bottom);

  friend class TextEditViewPrivate;
};

class LineNumberArea : public QWidget {
 public:
  LineNumberArea(TextEditView* editor) : QWidget(editor) { m_codeEditor = editor; }

  QSize sizeHint() const override { return QSize(m_codeEditor->lineNumberAreaWidth(), 0); }

 protected:
  void paintEvent(QPaintEvent* event) override { m_codeEditor->lineNumberAreaPaintEvent(event); }

 private:
  TextEditView* m_codeEditor;
};
