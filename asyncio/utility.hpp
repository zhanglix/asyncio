#pragma once

#include <experimental/coroutine>

BEGIN_ASYNCIO_NAMESPACE;

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
  void clear() {
    _ready = false;
    _caller = nullptr;
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
  T await_resume() const noexcept { return _value; }
  void resume(T value) {
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