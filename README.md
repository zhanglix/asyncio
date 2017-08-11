## What is AsyncIO?

AsyncIO is a C++ coroutine helper library based on llvm-5.0's (or above) coroutine feature. [AsycnIO v0.2](/zhanglix/asyncio/tree/v_0_2) has two components:

* [Couroutine](docs/coroutine.md) 
  Coroutine makes it much easier to write coroutine methods, generators and asynchronous generators. These part of AsyncIO is header only that means if this is all you need, you can just download the source code into your projects, then use it without build AsyncIO
* [EventLoop](docs/event_loop.md)
  EventLoop is a simple scheduler based on [libuv](http://libuv.org/), it helps you run multiple coroutines "simultaneously" within a thread.

### How to Run Build AsyncIO

#### On MacOS X
Make sure you have installed llvm 5.0 or above, libc++ and cmake. the simplest way to install it is use brew
```bash
brew tap homebrew/versions
brew install --HEAD llvm #this will install libc++ by default
brew install cmake

cd $ASYNCIO_PATH && mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release .. #there are some option in Debug config which conflict with -fcoroutines-ts that will cause clang crash.
make -j 10
make test
```
#### On Linux
TO BE ADD

## Examples
There are some examples in asyncio/examples directory. For more detail infomation about specific class, you can check the tests in asyncio/tests directory or just have a look at the code. 

### adds.cpp
Source Code: adds.cpp

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
compile it 

```bash
$ clang -I $ASYNCIO_HEADER_PATH -o adds -lc++ -std=c++14 -stdlib=libc++ -fcoroutines-ts -lasyncio -L $ASYNCIO_LIB_PATH -rpath $ASYNCIO_LIB_PATH adds.cpp
 
$ ./adds
add (1, 2) and (3, 4) first:
3 + 4 = 7
1 + 2 = 3
then add their sum:
3 + 7 = 10
Done! 
We get the total: 10!

```






