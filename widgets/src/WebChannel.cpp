#include "WebChannel.h"
#include "core/V8Util.h"

using core::V8Util;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;
using v8::Exception;
using v8::MaybeLocal;
using v8::ObjectTemplate;
using v8::Maybe;
using v8::Boolean;
using v8::Array;
using v8::Context;
using v8::Private;
using v8::External;
using v8::UniquePersistent;

WebChannelProxyObject::WebChannelProxyObject(QObject* parent) : QObject(parent) {}

WebChannelProxyObject::~WebChannelProxyObject() {
  qDebug() << "~WebChannelProxyObject";
}

void WebChannelProxyObject::receive(const QString& event, QVariant data) {
  emit onMessage(event, data);
}

void WebChannelProxyObject::initFinished() {
  emit connection();
}

WebChannel::WebChannel(QObject* parent)
    : QWebChannel(parent), m_proxyObject(new WebChannelProxyObject(this)) {
  registerObject("_silkedit_proxy", m_proxyObject);
  connect(m_proxyObject, &WebChannelProxyObject::connection, this,
          [this] { emit connection(this); });
  connect(m_proxyObject, &WebChannelProxyObject::onMessage,
          [this](const QString& event, QVariant data) {
            Isolate* isolate = Isolate::GetCurrent();
            v8::Locker locker(isolate);
            v8::HandleScope handle_scope(isolate);

            if (m_eventMap.count(event) != 0) {
              const UniquePersistent<Function>& listener = m_eventMap[event];
              Local<Value> argv[1];
              argv[0] = V8Util::toV8Value(isolate, data);
              V8Util::callJSFunc(isolate, listener.Get(isolate), v8::Undefined(isolate), 1, argv);
            }
          });
}

WebChannel::~WebChannel() {
  deregisterObject(m_proxyObject);
}

void WebChannel::sendMessage(const QString& event, bool data) {
  emit m_proxyObject->send(event, data);
}

void WebChannel::sendMessage(const QString& event, const QString& data) {
  emit m_proxyObject->send(event, data);
}

// pure JS object is converted to QVariantMap
void WebChannel::sendMessage(const QString& event, QVariantMap data) {
  emit m_proxyObject->send(event, data);
}

void WebChannel::sendMessage(const QString& event, QVariantList data) {
  emit m_proxyObject->send(event, data);
}

void WebChannel::sendMessage(const QString& event, int data) {
  emit m_proxyObject->send(event, data);
}

void WebChannel::sendMessage(const QString& event, double data) {
  emit m_proxyObject->send(event, data);
}

void WebChannel::onMessage(const QString& event, core::FunctionInfo fnInfo) {
  m_eventMap[event] = UniquePersistent<Function>(fnInfo.isolate, fnInfo.fn);
}
