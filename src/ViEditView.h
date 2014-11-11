#pragma once

#include <QPlainTextEdit>
#include <QObject>

#include "ICursorDrawer.h"

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QElapsedTimer;
QT_END_NAMESPACE

class LineNumberArea;

class ViEditView : public QPlainTextEdit {
  Q_OBJECT

 public:
  ViEditView(QWidget* parent = 0);
  ~ViEditView() = default;

  inline void setCursorDrawer(std::unique_ptr<ICursorDrawer> cursorDrawer) {
    m_cursorDrawer = std::move(cursorDrawer);
  }

  void lineNumberAreaPaintEvent(QPaintEvent* event);
  int lineNumberAreaWidth();
  void moveCursor(int mv, int = 1);
  void doDelete(int n);
  void doUndo(int n);
  void doRedo(int n);

 public slots:
  void updateCursor();

 protected:
  virtual void keyPressEvent(QKeyEvent* e);

  void resizeEvent(QResizeEvent* event);
  void paintEvent(QPaintEvent* e);
  void wheelEvent(QWheelEvent* event);
  void drawCursor();
  void setFontPointSize(int sz);
  void makeFontBigger(bool bigger);
  int firstNonBlankCharPos(const QString& text);
  bool isTabOrSpace(const QChar ch);
  void moveToFirstNonBlankChar(QTextCursor& cur);

 private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect&, int);

 private:
  QWidget* m_lineNumberArea;
  std::unique_ptr<ICursorDrawer> m_cursorDrawer;
};

class LineNumberArea : public QWidget {
 public:
  LineNumberArea(ViEditView* editor) : QWidget(editor) { m_codeEditor = editor; }

  QSize sizeHint() const { return QSize(m_codeEditor->lineNumberAreaWidth(), 0); }

 protected:
  void paintEvent(QPaintEvent* event) { m_codeEditor->lineNumberAreaPaintEvent(event); }

 private:
  ViEditView* m_codeEditor;
};
