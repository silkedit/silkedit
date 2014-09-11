#pragma once

#include <QPlainTextEdit>
#include <QObject>

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
QT_END_NAMESPACE

class LineNumberArea;

class ViEditView : public QPlainTextEdit {
  Q_OBJECT
 public:
  enum Mode {
    CMD = 0,
    INSERT,
  };

 public:
  ViEditView(QWidget *parent = 0);

  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();

  Mode mode() const { return m_mode; }
  void setMode(Mode mode) { m_mode = mode; }

 protected:
  void resizeEvent(QResizeEvent *event);
  void keyPressEvent(QKeyEvent *event);
  void cmdModeKeyPressEvent(QKeyEvent *);
  void insertModeKeyPressEvent(QKeyEvent *);

 private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect &, int);

 private:
  QWidget *m_lineNumberArea;
  Mode m_mode;
};

class LineNumberArea : public QWidget {
 public:
  LineNumberArea(ViEditView *editor) : QWidget(editor) {
    m_codeEditor = editor;
  }

  QSize sizeHint() const {
    return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
  }

 protected:
  void paintEvent(QPaintEvent *event) {
    m_codeEditor->lineNumberAreaPaintEvent(event);
  }

 private:
  ViEditView *m_codeEditor;
};
