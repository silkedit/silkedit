#pragma once

#include <unordered_map>
#include <v8.h>
#include <QDebug>
#include <QWebChannel>

#include "core/macros.h"
#include "core/FunctionInfo.h"
#include "core/stlSpecialization.h"

class WebChannelProxyObject : public QObject {
  Q_OBJECT
  DISABLE_COPY(WebChannelProxyObject)

 public:
  WebChannelProxyObject(QObject* parent);
  ~WebChannelProxyObject();
  DEFAULT_MOVE(WebChannelProxyObject)

 signals:
  void send(const QString& name, QVariant value);
  void onMessage(const QString& event, QVariant data);
  void connection();

 public slots:
  void receive(const QString& event, QVariant data);
  void initFinished();
};

class WebChannel : public QWebChannel {
  Q_OBJECT
  DISABLE_COPY(WebChannel)

 public:
  Q_INVOKABLE WebChannel(QObject* parent = nullptr);
  ~WebChannel();
  DEFAULT_MOVE(WebChannel)

  Q_INVOKABLE void sendMessage(const QString& event, bool data);
  Q_INVOKABLE void sendMessage(const QString& event, int data);
  Q_INVOKABLE void sendMessage(const QString& event, double data);
  Q_INVOKABLE void sendMessage(const QString& event, const QString& data);
  Q_INVOKABLE void sendMessage(const QString& event, QVariantMap data);
  Q_INVOKABLE void sendMessage(const QString& event, QVariantList data);
  Q_INVOKABLE void onMessage(const QString& event, core::FunctionInfo fnInfo);

 signals:
  void connection();

 private:
  WebChannelProxyObject* m_proxyObject;
  std::unordered_map<QString, v8::UniquePersistent<v8::Function>> m_eventMap;
};

Q_DECLARE_METATYPE(WebChannel*)
