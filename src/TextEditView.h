#pragma once

#include <memory>
#include <QObject>
#include <stextedit.h>

#include "macros.h"
#include "ICloneable.h"

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QElapsedTimer;
QT_END_NAMESPACE

class LineNumberArea;

class TextEditView : public STextEdit, public ICloneable<TextEditView> {
  Q_OBJECT

 public:
  explicit TextEditView(const QString& path = "", QWidget* parent = 0);
  virtual ~TextEditView();

  QString path() { return m_path; }
  void setDocument(std::shared_ptr<QTextDocument> document) {
    m_document = document;
    STextEdit::setDocument(document.get());
    updateLineNumberAreaWidth(blockCount());
  }

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

signals:
  void destroying(const QString& path);
  void pathUpdated(const QString& path);

 protected:
  void resizeEvent(QResizeEvent* event) override;
  void paintEvent(QPaintEvent* e) override;
  void wheelEvent(QWheelEvent* event) override;
  void setFontPointSize(int sz);
  void makeFontBigger(bool bigger);
  int firstNonBlankCharPos(const QString& text);
  bool isTabOrSpace(const QChar ch);
  void moveToFirstNonBlankChar(QTextCursor& cur);

 private:
  QWidget* m_lineNumberArea;
  QString m_path;
  std::shared_ptr<QTextDocument> m_document;

  void setPath(const QString& path);

 private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect&, int);
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
