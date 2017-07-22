#include <iostream>
#include <string>

#include <asyncio/coroutine.hpp>

using namespace std;
using namespace asyncio;

coro<int> identity(int x) { co_return x; }

co_gen<int> range(int n) {
  for (int i = 0; i < n; i++) {
    co_yield identity(i);
  }
}

coro<void> f() {
  for
    co_await(auto &&v : range(30)) { cout << v << " "; }
  cout << endl;
}

int main(int argc, char *arvg[]) {
  co_runner<void> cr(f());
  cr.get_future().get();
}