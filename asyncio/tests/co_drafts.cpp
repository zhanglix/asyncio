#include <iostream>

#include <experimental/coroutine>
using namespace std;
using namespace std::experimental;

// ::operator new(size_t, nothrow_t) will be used if allocation is needed

struct my_suspend_always {

  bool await_ready() const _NOEXCEPT {
    cout << "I am not ready!" << this << endl;
    return false;
  }

  void await_suspend(coroutine_handle<> h) const _NOEXCEPT {
    cout << "I am suspending!" << this << "in coroutine: " << h.address()
         << endl;
  }

  void await_resume() const _NOEXCEPT { cout << "I resumed!" << this << endl; }
};
struct generator {
  struct promise_type;
  using handle = coroutine_handle<promise_type>;
  struct promise_type {
    int current_value;
    promise_type() { cout << "Construct Promise: " << this << endl; }
    static auto get_return_object_on_allocation_failure() {
      cerr << "Failed to allocate return object" << endl;
      return generator{nullptr};
    }
    auto get_return_object() {
      cout << "get_return_object ..." << endl;
      return generator{handle::from_promise(*this)};
    }
    auto initial_suspend() {
      cout << "initial_suspend. promise:" << this << endl;
      return my_suspend_always{};
    }
    auto final_suspend() {
      cout << "final_suspend. promise:" << this << endl;
      return my_suspend_always{};
    }
    void return_void() { cout << "return void" << this << endl; }
    // void return_value(int value) { current_value = value; }
    void unhandled_exception() {
      cout << "unhandled_exception" << this << endl;
    }
    auto yield_value(int value) {
      current_value = value;
      return my_suspend_always{};
    }
    template <typename... Args>
    void *operator new(size_t size, Args const &...) noexcept {
      void *p = malloc(size);
      cout << "Allocated memory: " << p << " size: " << size << endl;
      return p;
    }

    void operator delete(void *p, size_t size) noexcept {
      free(p);
      cout << "Freed memory: " << p << " size: " << size << endl;
    }
  };
  bool move_next() {
    cout << "calling coro.resume() coro: " << coro.address() << endl;
    return coro ? (coro.resume(), !coro.done()) : false;
  }
  int current_value() { return coro.promise().current_value; }
  ~generator() {
    cout << "destory generator: " << (void *)this << " coro: " << coro.address()
         << endl;
    if (coro)
      coro.destroy();
  }
  generator(generator &&other) : coro(other.coro) {
    cout << "move construct generator: " << (void *)this << " from " << &other
         << " coro: " << coro.address() << endl;
    other.coro = nullptr;
  }
  generator(generator &) = delete;

private:
  generator(handle h) : coro(h) {
    cout << "construct generator: " << (void *)this
         << " coro: " << coro.address() << endl;
  }

  handle coro;
};
generator f() {
  co_yield 1;
  co_yield 2;
  throw 3;
}

int main() {
  cout << "calling f()..." << endl;
  auto g = f();
  cout << "returned from f(). g: " << &g << endl;
  while (g.move_next())
    cout << g.current_value() << endl;
  cout << "after move next. g: " << &g << endl;
}