## What is AsyncIO?

Currently, AsyncIO is a C++ coroutine helper libraryr based on llvm-5.0 (or above) coroutine feature to make it much easier to write coroutine methods, generator and asynchronous generators. 

## What is coroutine?
A coroutine is a special kind of _Functions_ whose execution can be 'paused' at some specific positions and resumed later.  Coroutines can make asynchronous event driven logic looks like synchronous, and has much lower overhead than multithreading synchronous implementation. 

A typical event-driven logic which based on callback mechanism usually looks like this:

```c++
// asynchronous callback based event-driven logic
void handleSearchQuery(Request &request) {
  Rewriter rewriter("rpc://localhost/queryrewriter");
  rewriter.rewrite(request, rewriteCallback);
}

void rewriteCallback(Request &request) {
  SearchService search("rpc://somehost/");
  search.search(request, searchCallback);
}

void searchCallback(Request &request, Response &response) {
  ResponseEndPoint endpoint;
  endpoint.reply(request, response, replyCallback);
}

void replyCallCallback(Request &request, int status) {
  if (status == Status::OK) {
    LOG("{} succeed!", request);
  } else {
    LOG_ERROR("{} failed! status: {}", request, status);
  }
}
```

and the same asynchronous logic can be rewrite with coroutines like this:
```c++
coro<void> handleSearch(Request &request) {
  try{
    Rewriter rewriter("rpc://localhost/queryrewriter");
    SearchService search("rpc://somehost/");
    ResponseEndPoint endpoint;

    co_await rewriter(request);
    Response response = co_await search.search(request);
    co_await = endpoint.reply(request, response);
    LOG("{} succeed!", request);
  }catch(std::exception &e){
    LOG_ERROR("{} failed! Error: {}", request, e.what());
  }
}
```

## How to use AsyncIO
### How to install
No, AsyncIO is a header only library, so all you need to do is download it and tell clang where to find it .

### Tutorial

#### How to write a coroutine method/function

Define a method/function whose return type is coro\<SomeType>, then you can use *co_await* and *co_return* in your method/function.

```c++
coro<float> pi() { co_return 3.14;}

coro<float> circleArea(float r) { 
  int p = co_await pi(); 
  co_return p * r * r;
}
```

Make use of AWaitable\<SomeType> helper class to wrap an callback based service into a corotine method.

```c++
coro<string> query(string request, Service &service) {
  AWaitable<string> awaitable;
  service.query(request, &awaitable, 
                [](string response, int error, void *userdata){
                  auto aw = (AWaitable *)userdata;
                  if (error == 0) {
                    aw->resume(response);
                  } else {
                    aw->setException(std::make_exception_ptr(error))
                  }
                });
  co_return co_await awaitable;
}
```

#### How to write a generator
A generator is a special kind of coroutine which can be used to generate a sequence of values. Using a generator can mimic the behavior of containers without relly store those elements. Typical usages of generator are in range-based for loops.

To define a generator, define the return type of your method as gen\<SomeType>, and use *co_yield* keyword to generator values.

Note: You *CAN NOT* use any co_await expression in generator

``` c++
gen<int> range(int n) {
  for (int i = 0; i < n; i++) {
    co_yield i;
  }
}

int main(int argc, char* argv[]) {
  for (auto &&v: range(10)) {
    cout << v << " ";
  }
  cout << endl;
}
//will output:
//0 1 2 3 4 5 6 7 8 9
```

#### How to write a asynchronous generator
A asynchronous generator is similary with generator except it *begin* and *operator++* method are coroutines, so that it can be used in a range-based for co_await loop.

To define a asynchronous generator, define the return type of your method as co_gen\<SomeType>, and use *co_yield* keyword to generator values of type coro<SomeType>.

Note: You *CAN NOT* use any co_await expression in asynchronous generator

``` c++
coro<int> identity(int x) {co_return x;}

co_gen<int> range(int n) {
  for (int i = 0; i < n; i++) {
    co_yield identity(i);
  }
}

coro<void> asyncTraversal(int n) {
  for co_await (auto &&v: range(n)) {
    cout << v << " ";
  }
  cout << endl;
}
```

#### How to run a coro\<Sometype> in normal function context
Since co_await can only exist in coroutine context, AsyncIO provides a helper class co_runner<Sometype> to simplify the interaction with coroutines.

```c++
extern coro<void> asyncTraversal; //defined above

int main(int argc, char* argv[]) {
  co_runner<void> cr(asyncTraversal(10));
  cr.get_future().get();
}
//will output:
//0 1 2 3 4 5 6 7 8 9
```

### Examples
There are some examples in asyncio/examples directory. For more detail infomation about specific class, you can check the tests in asyncio/tests directory or just have a look at the code. 

## Run AsyncIO Tests
### On MacOS X
Make sure you have installed llvm 5.0 or above, libc++ and cmake. the simplest way to install it is use brew
```bash
brew tap homebrew/versions
brew install --HEAD llvm #this will install libc++ by default
brew install cmake

cd $PATH_TO_ASYNC_IO && mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release .. #there are some option in Debug config which conflict with -fcoroutines-ts that will cause clang crash.
make -j 10
make test
```
### On Linux
TODO




