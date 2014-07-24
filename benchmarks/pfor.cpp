#include <hayai.hpp>
#include <vector>

#include "pfor/pfor.hpp"

template <unsigned int SleepMs>
void sleep_worker(int)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(SleepMs));
}

void sleep_worker_increasing(int i)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(i));
}

void sleep_worker_unbalanced(int i)
{
  std::this_thread::sleep_for(std::chrono::milliseconds((7 * i + 3) % 997));
}

BENCHMARK(SleepWorkerBalanced, Interleaved, 1, 10)
{
  p::pfor(p::launch::interleaved, 0, 500, sleep_worker<1>);
}

BENCHMARK(SleepWorkerBalanced, Block, 1, 10)
{
  p::pfor(p::launch::block, 0, 500, sleep_worker<1>);
}

BENCHMARK(SleepWorkerIncreasing, Interleaved, 1, 10)
{
  p::pfor(p::launch::interleaved, 0, 30, sleep_worker_increasing);
}

BENCHMARK(SleepWorkerIncreasing, Block, 1, 10)
{
  p::pfor(p::launch::block, 0, 30, sleep_worker_increasing);
}

BENCHMARK(SleepWorkerUnbalanced, Interleaved, 1, 10)
{
  p::pfor(p::launch::interleaved, 0, 20, sleep_worker_unbalanced);
}

BENCHMARK(SleepWorkerUnbalanced, Block, 1, 10)
{
  p::pfor(p::launch::block, 0, 20, sleep_worker_unbalanced);
}

BENCHMARK(SumWorker, Interleaved, 1, 100)
{
  std::vector<int> vec(1000000);
  std::iota(vec.begin(), vec.end(), 0);

  std::atomic<int> sum(0);
  p::pfor<int>(p::launch::interleaved, 0ul, vec.size(),
      [](int) { return 0; },
      [&](std::size_t i, int local) { return local + vec[i]; },
      [&](int tid, int local) { sum += local; });
}

BENCHMARK(SumWorker, Block, 1, 100)
{
  std::vector<int> vec(1000000);
  std::iota(vec.begin(), vec.end(), 0);

  std::atomic<int> sum(0);
  p::pfor<int>(p::launch::block, 0ul, vec.size(),
      [](int) { return 0; },
      [&](std::size_t i, int local) { return local + vec[i]; },
      [&](int tid, int local) { sum += local; });
}
