#pragma once

#include <QWebEngineView>

#include "core/macros.h"

class WebView : public QWebEngineView {
  Q_OBJECT
  DISABLE_COPY(WebView)

 public:
  Q_INVOKABLE WebView(QWidget* parent = nullptr);
  ~WebView() = default;
  DEFAULT_MOVE(WebView)

  public slots:
    void load(const QUrl& url);

 private:
};
