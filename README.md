# What is AsyncIO

AsyncIO is a C++ coroutine helper library based on llvm-5.0's (or above) coroutine feature. [AsycnIO v0.2](https://github.com/zhanglix/asyncio/tree/v_0_2) has two components:

* [Couroutine](docs/coroutine.md)
  Coroutine makes it much easier to write coroutine methods, generators and asynchronous generators. These part of AsyncIO is header only that means if this is all you need, you can just download the source code into your projects, then use it without build AsyncIO
* [EventLoop](docs/event_loop.md)
  EventLoop is a simple scheduler based on [libuv](http://libuv.org/), inspired by [EventLoop of Python3](https://docs.python.org/3/library/asyncio-eventloop.html#asyncio.AbstractEventLoop). it helps you run multiple coroutines "simultaneously" within a thread.

## Build Status

| Stage |OSX |
|:--:|:--:|
|Unit Test|[![Build Status](https://travis-ci.com/zhanglix/asyncio.svg?branch=master)](https://travis-ci.com/zhanglix/asyncio)||

## How to Install

### install dependency

Make sure you have installed llvm 5.0 or above, libc++, libc++abi, libuv and cmake.

#### On MacOS X

```bash
brew tap homebrew/versions
brew install --HEAD llvm #this will install libc++ by default
brew install cmake
brew install
```

#### On Ubuntu

```bash
sudo apt-get install libuv cmake
# install llvm-5.0 libc++ libc++abi to /usr/local
# download from here http://releases.llvm.org/download.html#5.0.0
```

### Build and Install

```bash
cd $ASYNCIO_PATH && mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 10
make test
make install
```

## Examples

There are some examples in asyncio/examples directory. For more detail infomation about specific class, you can check the tests in asyncio/tests directory or just have a look at the code.

### sleep_sort.cpp

Source Code: sleep_sort.cpp

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

compile it

```bash
$ clang -I $ASYNCIO_HEADER_PATH -o sleep_sort -lc++ -std=c++14 -stdlib=libc++ -fcoroutines-ts -lasyncio -L $ASYNCIO_LIB_PATH -rpath $ASYNCIO_LIB_PATH sleep_sort.cpp

$ ./sleep_sort
Someone says 'sleep sort' is O(1)?
1
3
5
7
9
... I think it makes sense! 🤣🤣🤣

```
