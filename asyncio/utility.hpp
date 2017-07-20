#pragma once

#include <experimental/coroutine>

#include "coro.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <typename ReturnType> class co_runner {
public:
  co_runner(coro<ReturnType> &co) : _runner(run(co)) {
    _runner.await_suspend(nullptr);
  }
  co_runner(coro<ReturnType> &&co) : _runner(run(co)) {
    _runner.await_suspend(nullptr);
  }
  co_runner(co_runner &) = delete;
  co_runner(co_runner &&) = delete;
  std::future<ReturnType> get_future() { return std::move(_future); }

private:
  coro<void> _runner;
  std::future<ReturnType> _future;

  coro<void> run(coro<ReturnType> &co) {
    std::promise<ReturnType> promise;
    this->_future = promise.get_future();
    try {
      ReturnType ret = co_await std::move(co);
      promise.set_value(ret);
    } catch (...) {
      promise.set_exception(std::current_exception());
    }
  }
};

template <> coro<void> inline co_runner<void>::run(coro<void> &co) {
  std::promise<void> promise;
  this->_future = promise.get_future();
  try {
    coro<void> co_holder = std::move(co);
    co_await co_holder;
    promise.set_value();
  } catch (...) {
    promise.set_exception(std::current_exception());
  }
}

class AWaitableBase {
public:
  AWaitableBase() : _ready(false), _caller(nullptr) {}

  bool await_ready() const noexcept { return _ready; }
  bool await_suspend(std::experimental::coroutine_handle<> caller) noexcept {
    if (!_ready) {
      _caller = caller;
    }
    return !_ready;
  }

protected:
  // void await_resume() const noexcept {}
  void setReady() { _ready = true; }
  void resumeCaller() {
    if (_caller) {
      _caller.resume();
    }
  }

  bool _ready;
  std::experimental::coroutine_handle<> _caller;
};

template <typename T> class AWaitable : public AWaitableBase {
public:
  T await_resume() const noexcept { return std::move(_value); }
  void resume(T &&value) {
    _value = std::move(value);
    this->setReady();
    this->resumeCaller();
  }
  void resume(T &value) {
    _value = value;
    this->setReady();
    this->resumeCaller();
  }

private:
  T _value;
};

template <> class AWaitable<void> : public AWaitableBase {
public:
  void await_resume() const noexcept {}
  void resume() {
    this->setReady();
    this->resumeCaller();
  }
};

END_ASYNCIO_NAMESPACE;