#include "WebView.h"

WebView::WebView(QWidget* parent) : QWebEngineView(parent) {}

void WebView::load(const QUrl &url)
{
  QWebEngineView::load(url);
}
