#include <iostream>
#include <rigtorp/MPMCQueue.h>
#include <thread>

int main(int argc, char *argv[]) {
  (void)argc, (void)argv;

  using namespace rigtorp;

  MPMCQueue<int> q(10);
  auto t1 = std::thread([&] {
    for (int i = 0; i < 10; ++i) {
      q.emplace(i);
      //   q.push(i);
      //   std::cout << "t1 " << i << "\n";
    }
  });

  auto t2 = std::thread([&] {
    int v;
    for (int i = 0; i < 10; ++i) {
      //   q.pop(v);
      q.try_pop(v);
      std::cout << "t2 " << v << "\n";
    }
  });

  t1.join();
  t2.join();

  return 0;
}
