#include <asyncio/asyncio.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace asyncio;

coro<string> runner(EventLoop *loop, string name, uint64_t needTime) {
  co_await sleep(loop, needTime);
  co_return name;
}

coro<void> race(EventLoop *loop) {
  vector<FutureBase*> futs{
    loop->createTask(runner(loop, "Foo", 10)),
    loop->createTask(runner(loop, "Goo", 5)),
    loop->createTask(runner(loop, "Hoo", 1)),
  };
  
  int idx = 0;
  string winner;
  cout << "Who runs fast?" << endl;
  for co_await(auto && fut: FutureCoGen(futs)) {
    idx ++;
    string name = ((Future<string>*)fut)->get();
    cout << idx << ": " << name << endl; 
    if(idx == 1) {
      winner = name;
    }
    fut->release();
  }
  cout << "The winner is -- " << winner << "!" << endl;
}

int main(int argc, char *argv[]) {
  EventLoop loop;
  auto task = loop.createTask(race(&loop));
  loop.runUntilDone(task);
  task->release();
}
