#include <asyncio/asyncio.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace asyncio;

coro<int> addLater(EventLoop *loop, int a, int b, uint64_t ms = 0) {
  co_await sleep(loop, ms);
  cout << a << " + " << b << " = " << a + b << endl;
  co_return a + b;
}

coro<void> adds(EventLoop *loop) {
  auto &&add1 = addLater(loop, 1, 2, 10);
  auto &&add2 = addLater(loop, 3, 4);
  cout << "add (1, 2) and (3, 4) first:" << endl;
  auto sums = co_await all(loop, add1, add2);
  cout << "then add their sum:" << endl;
  auto total = co_await addLater(loop, get<0>(sums), get<1>(sums));
  cout << "Done! " << endl;
  cout <<"We get the total: " <<  total << "!" << endl;
}

int main(int argc, char *argv[]) {
  EventLoop loop;
  auto task = loop.createTask(adds(&loop));
  loop.runUntilDone(task);
  task->release();
}
