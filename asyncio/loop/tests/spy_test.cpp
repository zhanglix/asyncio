#include <catch.hpp>
#include <fakeit.hpp>

using namespace fakeit;

namespace spy_test {
class SomeClass {
public:
  virtual int func1(int arg) { return arg; }
  virtual int func2(int arg) { return arg; }
};

TEST_CASE("spy test", "[fakeit]") {

  SomeClass obj;
  Mock<SomeClass> spy(obj);

  When(Method(spy, func1)).AlwaysReturn(10); // Override to return 10
  Spy(Method(spy, func2));
  SomeClass &i = spy.get();
  SECTION("verify func1") {
    CHECK(i.func1(1) == 10);
    Verify(Method(spy, func1)).Once();
  }
  SECTION("verify func2") {
    CHECK(i.func2(1) == 1);
    Verify(Method(spy, func2)).Once();
  }
}
} // namespace spy_test