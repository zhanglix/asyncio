#include <iostream>
#include <string>
#include <asyncio/gen.hpp>

using namespace std;
using namespace asyncio;

gen<int> range(int n) {
  for (int i = 0; i < n; i++) {
    co_yield i;
  }
}
int main(int argc, char *arvg[]){
  for (auto &&v: range(30)){
    cout << v << " ";
  }
  cout << endl;
}