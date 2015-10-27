#pragma once

namespace core {

// https://ezoeryou.github.io/blog/article/2014-10-21-finally.html
// RAII for anything. This is the replacement for finally block in Java
// Similar to defer in Golang
class scoped_guard {
  std::function<void()> f;

 public:
  explicit scoped_guard(std::function<void()> f) : f(f) {}

  scoped_guard(scoped_guard const&) = delete;
  void operator=(scoped_guard const&) = delete;

  ~scoped_guard() { f(); }
};

}  // namespace core
