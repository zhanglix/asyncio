#include <asyncio/asyncio.hpp>
#include <iostream>

using namespace std;
using namespace asyncio;

coro<void> sleepCout(EventLoop *loop, uint64_t value) {
  co_await sleep(loop, value);
  cout << value << endl;
}

int main() {
  EventLoop loop;
  cout << "Someone says 'sleep sort' is O(1)?" << endl;
  uint64_t max = -1;
  for (auto &&v : {5, 1, 9, 7, 3}) {
    loop.createTask(sleepCout(&loop, v))->release();
    max = max > v ? max : v;
  }
  loop.callLater(max + 20, [&] { loop.stop(); })->release();
  loop.runForever();
  cout << "... I think it makes sense! 🤣🤣🤣" << endl;
}