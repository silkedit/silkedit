#include <QQuickWidget>
#include <QDebug>
#include <QQuickItem>

#include "UpdateNotificationView.h"
#include "App.h"
#include "Window.h"

void UpdateNotificationView::show() {
  auto widget = new QQuickWidget;
  widget->setFocusPolicy(Qt::NoFocus);
  connect(widget, &QQuickWidget::sceneGraphError,
          [](QQuickWindow::SceneGraphError, const QString& message) { qWarning() << message; });
  QUrl url("qrc:/qmls/notification.qml");
  widget->setSource(url);

  auto window = App::instance()->activeMainWindow();
  if (window) {
    widget->setParent(window);

    // move QQuickWidget to the top center of its parent window
    auto x = window->rect().center().x() - widget->rect().center().x();
    widget->move(x, 0);
  }

  QQuickItem* rootObj = widget->rootObject();
  Q_ASSERT(rootObj);

  auto laterButton = rootObj->findChild<QObject*>("laterButton");
  Q_ASSERT(laterButton);
  connect(laterButton, SIGNAL(clicked()), widget, SLOT(close()));

  auto updateButton = rootObj->findChild<QObject*>("updateButton");
  Q_ASSERT(updateButton);
  connect(updateButton, SIGNAL(clicked()), App::instance(), SLOT(restart()));

  widget->show();
}
