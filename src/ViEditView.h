#pragma once

#include <QPlainTextEdit>
#include <QObject>

#include "vi.h"

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
QT_END_NAMESPACE

class LineNumberArea;
class ViEngine;
class QElapsedTimer;

class ViEditView : public QPlainTextEdit {
  Q_OBJECT

 public:
  ViEditView(QWidget* parent = 0);
  ~ViEditView();

  Mode mode() const { return m_mode; }

  void lineNumberAreaPaintEvent(QPaintEvent* event);
  int lineNumberAreaWidth();
  void moveCursor(int mv, int = 1);
  void doDelete(int n);
  void doUndo(int n);
  void doRedo(int n);
  void evalRuby(const QString& str);

 public slots:
  void setMode(Mode mode);
  void onCursorPositionChanged();

 protected:
  void resizeEvent(QResizeEvent* event);
  void paintEvent(QPaintEvent* e);
  void wheelEvent(QWheelEvent* event);
  void drawCursor();
  void setFontPointSize(int sz);
  void makeFontBigger(bool bigger);
  int firstNonBlankCharPos(const QString& text);
  bool isTabOrSpace(const QChar ch);
  void moveToFirstNonBlankChar(QTextCursor& cur);

signals:
  void modeChanged(Mode);

 private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect&, int);

 private:
  Mode m_mode;
  QWidget* m_lineNumberArea;
  int m_cursorWidth;
  qint64 m_tickCount;
  QElapsedTimer* m_timer;
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
