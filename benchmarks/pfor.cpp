#include <hayai.hpp>

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
  p::pfor(0, 500, sleep_worker<1>, p::launch::interleaved);
}

BENCHMARK(SleepWorkerBalanced, Block, 1, 10)
{
  p::pfor(0, 500, sleep_worker<1>, p::launch::block);
}

BENCHMARK(SleepWorkerBalanced, Dynamic, 1, 10)
{
  p::pfor(0, 500, sleep_worker<1>, p::launch::dynamic);
}

BENCHMARK(SleepWorkerIncreasing, Interleaved, 1, 10)
{
  p::pfor(0, 30, sleep_worker_increasing, p::launch::interleaved);
}

BENCHMARK(SleepWorkerIncreasing, Block, 1, 10)
{
  p::pfor(0, 30, sleep_worker_increasing, p::launch::block);
}

BENCHMARK(SleepWorkerIncreasing, Dynamic, 1, 10)
{
  p::pfor(0, 30, sleep_worker_increasing, p::launch::dynamic);
}

BENCHMARK(SleepWorkerUnbalanced, Interleaved, 1, 10)
{
  p::pfor(0, 20, sleep_worker_unbalanced, p::launch::interleaved);
}

BENCHMARK(SleepWorkerUnbalanced, Block, 1, 10)
{
  p::pfor(0, 20, sleep_worker_unbalanced, p::launch::block);
}

BENCHMARK(SleepWorkerUnbalanced, Dynamic, 1, 10)
{
  p::pfor(0, 20, sleep_worker_unbalanced, p::launch::dynamic);
}
