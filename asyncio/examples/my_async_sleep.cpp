#include <chrono>
#include <iostream>
#include <thread>

#include <asyncio/coro.hpp>
#include <asyncio/utility.hpp>

using namespace std;
using namespace std::chrono;
using namespace asyncio;

coro<void> my_async_sleep(milliseconds ms) {
  AWaitable<void> awaitable;
  thread t([&] {
    // coro<> is not thread safe, for producetion code
    // it is recommended to base coro<> on some single threading
    // callback library such as libuv.
    cout << "sleep ..." << endl;
    this_thread::sleep_for(ms);
    cout << "After sleep!" << endl;
    awaitable.resume();
  });
  t.detach();
  co_await awaitable;
}

coro<void> run() {
  cout << "start run!" << endl;
  cout << "before first sleep" << endl;
  co_await my_async_sleep(100ms);
  cout << "before second sleep" << endl;
  co_await my_async_sleep(200ms);
  cout << "end run!" << endl;
}

int main(int argc, char *argv[]) {
  co_runner<void> cr(run());
  cout << "waiting run() to finish" << endl;
  cr.get_future().get();
  cout << "run() finished!" << endl;
}
