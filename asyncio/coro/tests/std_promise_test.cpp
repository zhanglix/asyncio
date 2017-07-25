#include <catch.hpp>
#include <future>

TEST_CASE("promise", "[std::promise]") {
  auto p = new std::promise<int>();
  auto future = p->get_future();
  CHECK(future.valid());
  CHECK_THROWS_AS(p->get_future(), std::future_error);
  p->set_value(10);
  delete p;
  CHECK(future.valid());
  CHECK(future.get() == 10);
}