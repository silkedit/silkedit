#pragma once

#include <QWebEngineView>

#include "core/macros.h"

class WebEngineView : public QWebEngineView {
  Q_OBJECT
  DISABLE_COPY(WebEngineView)

 public:
  Q_INVOKABLE WebEngineView(QWidget* parent = nullptr);
  ~WebEngineView() = default;
  DEFAULT_MOVE(WebEngineView)

  public slots:
    void load(const QUrl& url);

 private:
};
