#include <catch.hpp>
#include <functional>
#include <type_traits>
#include <utility>

using namespace std;

namespace pack_test {

int foo(int a, int b, int c) { return a; }
void doo(int in, int &out) { out = in; }

template <class First, class... Args> First goo(First f, Args... args) {
  return f;
}

template <class R, class... Args> R callSoon(R (*f)(Args...), Args... args) {
  return (*f)(args...);
}

template <class R, class... Args>
R callSoon(function<R(Args...)> f, Args... args) {
  return f(args...);
}

TEST_CASE("parameter_pack", "[drafts]") {
  CHECK(callSoon(foo, 2, 3, 4) == foo(2, 3, 4));
  CHECK(callSoon(goo<int, char>, 4, 'c') == goo(4, 'c'));
  function<int(int, char)> f = [](int i, char c) { return i; };
  CHECK(callSoon(f, 4, 'c') == f(4, 'c'));
  int out = 0;
  callSoon<void, int, int &>(doo, 10, out);
  CHECK(out == 10);
  // vector<int> v;
  // v.push_back(3);
  // auto ret = callSoon<vector<int>, vector<int>>([](vector<int> v) { return v;
  // },
  //                                               vector<int>(1, 3));
  // CHECK(ret == v);
}

template <class F, class... Args>
typename result_of<F(Args...)>::type callSoon2(F &&f, Args &&... args) {
  return forward<F>(f)(forward<Args>(args)...);
}

TEST_CASE("parameter_pack2", "[drafts]") {
  CHECK(callSoon2(foo, 2, 3, 4) == foo(2, 3, 4));
  CHECK(callSoon2(goo<int, char>, 4, 'c') == goo(4, 'c'));
  function<int(int, char)> f = [](int i, char c) { return i; };
  CHECK(callSoon2(f, 4, 'c') == f(4, 'c'));

  int out = 0;
  callSoon2(doo, 10, out);
  CHECK(out == 10);
  vector<int> v;
  v.push_back(3);
  auto ret = callSoon2([](vector<int> v) { return v; }, vector<int>(1, 3));
  CHECK(ret == v);
}

template <class R, class F> class CallableTyped {
public:
  CallableTyped(F &f) : _f(f) {}
  R operator()() { return _f(); }
  F _f;
};

template <class F, class... Args>
typename result_of<F(Args...)>::type callSoon3(F &&f, Args &&... args) {

  //  auto b = [&] { return forward<F>(f)(forward<Args>(args)...); };
  auto b = bind(forward<F>(f), forward<Args>(args)...);
  auto callable =
      CallableTyped<typename result_of<F(Args...)>::type, decltype(b)>(b);
  return callable();
}

TEST_CASE("parameter_pack3", "[drafts]") {
  CHECK(callSoon3(foo, 2, 3, 4) == foo(2, 3, 4));
  CHECK(callSoon3(goo<int, char>, 4, 'c') == goo(4, 'c'));
  function<int(int, char)> f = [](int i, char c) { return i; };
  CHECK(callSoon3(f, 4, 'c') == f(4, 'c'));

  int out = 0;
  callSoon3(doo, 10, ref(out));
  CHECK(out == 10);
  vector<int> v;
  v.push_back(3);
  auto ret = callSoon3([](vector<int> v) { return v; }, vector<int>(1, 3));
  CHECK(ret == v);
}

} // namespace pack_test