#pragma once

#include <QDebug>
#include <QWebEngineView>

#include "WebPage.h"
#include "core/macros.h"

class WebView : public QWebEngineView {
  Q_OBJECT
  DISABLE_COPY(WebView)

 public:
  Q_INVOKABLE WebView(QWidget* parent = nullptr) : QWebEngineView(parent) {}
  ~WebView() { qDebug() << "~WebView"; }
  DEFAULT_MOVE(WebView)

 public slots:
  void load(const QString& url) { QWebEngineView::load(QUrl(url)); }

  void setHtml(const QString& html, const QString& baseUrl = "") {
    QWebEngineView::setHtml(html, QUrl(baseUrl));
  }

  void setPage(WebPage* page) { QWebEngineView::setPage(page); }
};
