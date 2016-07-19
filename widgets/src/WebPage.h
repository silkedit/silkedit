#pragma once

#include <QWebEnginePage>

#include "WebChannel.h"
#include "core/macros.h"

class WebPage : public QWebEnginePage {
  Q_OBJECT
  DISABLE_COPY(WebPage)

 public:
  Q_INVOKABLE WebPage(QObject* parent = nullptr) : QWebEnginePage(parent) {}
  ~WebPage() = default;
  DEFAULT_MOVE(WebPage)

 public slots:
  void load(const QString& url) { QWebEnginePage::load(QUrl(url)); }

  void setHtml(const QString& html, const QString& baseUrl = "") {
    QWebEnginePage::setHtml(html, QUrl(baseUrl));
  }

  void setWebChannel(WebChannel* channel) { QWebEnginePage::setWebChannel(channel); }
};

Q_DECLARE_METATYPE(WebPage*)
