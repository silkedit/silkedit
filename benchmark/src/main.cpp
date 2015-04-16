#include <QFile>
#include <QTextDocument>
#include <QApplication>
#include <QTime>

#include "Theme.h"
#include "Session.h"
#include "SyntaxHighlighter.h"

int main(int argv, char** args) {
  Theme* theme = Theme::loadTheme("testdata/Monokai.tmTheme");

  const QVector<QString> files({"testdata/C.tmLanguage", "testdata/C++.tmLanguage"});

  foreach (QString fn, files) { LanguageProvider::loadLanguage(fn); }

  QFile file("testdata/syntaxHighlightBenchmark.cpp");
  file.open(QIODevice::ReadOnly | QIODevice::Text);
  QTextStream in(&file);

  QTextDocument* doc = new QTextDocument(in.readAll());
  LanguageParser* parser = LanguageParser::create("source.c++", doc->toPlainText());
  Session::singleton().setTheme(theme);
  QTime t;
  t.start();
  new SyntaxHighlighter(doc, parser);
  qDebug("elapsed: %d ms", t.elapsed());
}
