#pragma once

#include <memory>
#include <QObject>
#include <QPlainTextEdit>

#include "macros.h"
#include "ICloneable.h"
#include "SyntaxHighlighter.h"
#include "Document.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QElapsedTimer;

class LineNumberArea;

class TextEditView : public QPlainTextEdit, public ICloneable<TextEditView> {
  Q_OBJECT

 public:
  explicit TextEditView(QWidget* parent);
  virtual ~TextEditView();

  QString path();
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
  void findText(const QString&, QTextDocument::FindFlags flags);
  void highlightSearchMatches(const QString& text);
  void clearSearchHighlight();

signals:
  void destroying(const QString& path);
  void pathUpdated(const QString& path);
  void saved();
  void languageChanged(const QString& scope);

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
  std::shared_ptr<Document> m_document;
  QVector<Region> m_searchMatchedRegions;


 private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect&, int);
  void changeTheme(Theme* theme);
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
