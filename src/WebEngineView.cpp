#include "WebEngineView.h"

WebEngineView::WebEngineView(QWidget* parent) : QWebEngineView(parent) {}

void WebEngineView::load(const QUrl &url)
{
  QWebEngineView::load(url);
}
