#pragma once

#include <v8.h>
#include <string>
#include <QString>

inline QString toQString(v8::Local<v8::String> str) {
  return QString::fromUtf16(*v8::String::Value(str));
}

inline std::string toStdString(v8::Local<v8::String> str) {
  v8::String::Utf8Value value(str);
  return *value;
}

inline v8::Local<v8::String> toV8String(
                                        v8::Isolate* isolate, const QString& str) {
  return v8::String::NewFromTwoByte(isolate, str.utf16());
}

inline v8::Local<v8::String> toV8String(
                                        v8::Isolate* isolate, const std::string& str) {
  return v8::String::NewFromUtf8(isolate, str.c_str());
}
