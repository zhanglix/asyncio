#include <asyncio/loop.hpp>
#include <iostream>
#include <thread>

using namespace std;
using namespace asyncio;
int add(int a, int b) { return a + b; }

int main() {
  EventLoop loop;
  auto fut = loop.callSoon(add, 3, 7);
  auto fut2 = loop.callLater(1000, add, 50, 50);

  thread t([&] {
    auto fut3 = loop.callSoonThreadSafe(add, 10, 20);
    cout << "10 + 20 = " << fut3->get() << endl;
    fut3->release();
  });
  t.detach();

  loop.runUntilDone(fut); // will return immediately
  cout << "3 + 7 = " << fut->get() << endl;

  loop.runUntilDone(fut2); // will return after about 1000ms
  cout << "50 + 50 = " << fut2->get() << endl;

  fut->release();  // should not use fut anymore1
  fut2->release(); // should not use fut2 anymore!
}