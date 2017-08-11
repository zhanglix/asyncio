## What are Coroutines?
Coroutines are of a special kind of _Functions_ whose execution can be 'paused' at some specific positions and be resumed at the paused position later. With Coroutines asynchronous codes can be written like synchronous codes, and has much lower overhead than multithreading synchronous implementation. 

A typical asynchronous event-driven logic which based on callback mechanism usually looks like this:

```c++
// asynchronous callback based event-driven logic
void handleSearchQuery(Request &request) {
  Rewriter rewriter("rpc://rewriter/");
  rewriter.rewrite(request, rewriteCallback);
}

void rewriteCallback(Request &request) {
  SearchService search("rpc://search/");
  search.search(request, searchCallback);
}

void searchCallback(Request &request, Response &response) {
  ResponseEndPoint endpoint;
  endpoint.reply(request, response, replyCallback);
}

void replyCallCallback(Request &request, int status) {
  if (status == Status::OK) {
    LOG_INFO("{} succeed!", request);
  } else {
    LOG_ERROR("{} failed! status: {}", request, status);
  }
}
```

and the same asynchronous logic can be rewritten with coroutines like this:
```c++
// coroutine based asynchronous event-driven logic,
// you can try catch exceptions in asynchronous codes as if it is synchronous. Amazing!
coro<void> handleSearch(Request &request) {
  try{
    Rewriter rewriter("rpc://rewriter/");
    SearchService search("rpc://search/");
    ResponseEndPoint endpoint;

    co_await rewriter.rewrite(request);
    Response response = co_await search.search(request);
    co_await endpoint.reply(request, response);
    LOG_INFO("{} succeed!", request);
  }catch(std::exception &e){
    LOG_ERROR("{} failed! Error: {}", request, e.what());
  }
}
```

## HOWTOs

Now, AsyncIO provides three kinds of coroutine convinient classes to write coroutines. 
* coro\<SomeType> 
* gen\<SomeType>
* co_gen\<SomeType>

### How to write a coroutine method/function

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
                    aw->throw(runtime_error("some error"));
                  }
                });
  co_return co_await awaitable;
}
```

### How to write a generator
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

### How to write a asynchronous generator
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

### How to run a coro\<Sometype> in normal function context
#### using EventLoop::createTask() recommended way 

```c++
extern coro<void> asyncTraversal; //defined above

int main(int argc, char* argv[]) {
  EventLoop loop;
  auto task = loop.createTask(asyncTraversal(10));
  loop.runUntilDone(task);
  task->release();
}
//will output:
//0 1 2 3 4 5 6 7 8 9
```

#### using co_runner\<SomeType>
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

### How to Customize Memory Allocation
You can think of a coroutine just as a normal function with most of its local variables stored in a state object which should be allocated in heap. For each instance (call) of coroutine, the compiler will generate codes to allocate memory for that instance before it is created, and codes to recycle memory of after its destruction. The default method for allocating/recycling is the standard *operator new()* and *operator delete* methods. 

For some performance critical portion of the code, the standard approach to allocate/recycle state object maybe unacceptable. There are two ways you can do to improve it.

First, you can use a better malloc implementation, such as tcmalloc, or Lockless Memory Allocator. 

Second, you can provide you own allocator for each coroutine method through the second template paramenter of class coro<>, gen<>, co_gen<>. For example:
```c++
class MyAllocator{
  void *operator new(size_t size) noexcept {
    void *p = Arena::getDefaultArena().allocate(size);
    return p;
  }
  void operator delete(void *p, size_t size) noexcept {
    Arena::deallocate(p);
  }
};

coro<int, MyAllocator> identity(int x) {co_return x;}

```

for more details, see [allocator_test.cpp](asyncio/tests/allocator_test.cpp)







