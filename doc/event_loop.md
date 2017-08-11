## EventLoop
We can think of coroutines as threads in multithreading programming except that coroutines can be _switched_ at predefined positions in the executation whereas threads can be _switched_ at any positions. To make most use of multpile cpus cores, we need the os kernel to shedule threading on/off at proper time. Similary, we need a scheduler to scheduling the running of multiple coroutines to make most use of a signle cpu.

AsyncIO provide a EventLoop class to schedule multiple coroutines run "simultaneous". 

EventLoop tries to provide similar functionalities of the [asyncio.AbstractEventLoop of Python3](https://docs.python.org/3/library/asyncio-eventloop.html#asyncio.AbstractEventLoop). Currently, Eventloop only provides the Scheduling related functionalities, and all the IO related methods has not implemented now.

Neverless, if your using libuv in your projects, it is relatively easy to wrapper your libuv based IO into coroutines and make use of AsyncIO's UVLoopCore class to integrate the _uv_loop_t_ object with EventLoop. Then you can use EventLoop to schedule all your libuv based IO coroutines.

```c++
uv_loop_t uvloop;
uv_loop_init(&uvloop);
//your own initialaztions 
//.....

EventLoop loop(new UVLoopCore(&uvloop));

//using loop ...
auto task = loop.createTask(myCoroutine());
loop.runUntilDone(task)
auto result = task->get();
task->release();
```

### Future\<SomeType> Class
All the scheduling related method of EventLoop will return a pointer to a Future\<SomeType> object. You can use the Future object to check if the coressponding task has done, get the result of the task, declare you will not use the Future object anymore.

The interface of Future class looks like these:

```c++
template<class T>
class Future<T> {
public:
  void release(); // release the reference to the undlerlying resource
  bool done(); // check if the corresponding task has done.
  T get(); // get/wait the result of the corresponding task
};
```

### Schedule Normal Function Calls
you can schedule normal functions be called soon or later.
```c++
#include <asyncio/loop.hpp>
#include <iostream>

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
```

### Schedule Coroutines
you can schedule Coroutines to run soon or later.
```c++
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
```

### About Thread Safety
Keep in mind that EventLoop and Coroutines are not thread safe, you can only call most of the mehtods of an EventLoop in the thread where you call the runUntilDone() methods. and schedule coroutines which will be _resumed_ by the same thread too. Sometimes you may need to submit some function call or coroutines from another thread, then need need to use ThreadSafe version of method.

### Wait Multiple Coroutine 
Sometimes we need to start two IO operation, and wait both of them to return. If we simply _co_await_ the first IO, then _co_await_ the second one, the latency of these two operations will be added. With the help of EventLoop, AsyncIO provides some simple functions/Class to wait schedule two IO simultaneously then wait all/any of them to finish.

#### Wait All Coroutine
```c++
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
```
#### Wait Any of Futures
```c++
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
```

