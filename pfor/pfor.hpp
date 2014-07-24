#pragma once

#include <thread>
#include <array>
#include <future>
#include <algorithm>
#include <chrono>

#ifndef PFOR_NTHREADS
#define PFOR_NTHREADS (4)
#endif

#ifndef PFOR_POLL_MS
#define PFOR_POLL_MS (10)
#endif

namespace p
{
  enum class launch
  {
    block,
    interleaved,
  };

  namespace detail
  {
    template <class T>
    void no_op(int, const T &) { }

    template <class Index, class Func>
    void work_stepped(Index from, Index to, Index step, const Func &f)
    {
      for (; from < to; from += step) f(from);
    }

    template <class Index, class Func, class InitFunc, class ReduceFunc>
    void work_stepped_local(int tid, Index from, Index to, Index step,
        const Func &f, const InitFunc &init, const ReduceFunc &reduce)
    {
      auto local = init(tid);
      for (; from < to; from += step) local = f(from, local);
      reduce(tid, local);
    }

    template <class Container>
    inline void join_all(Container &c)
    {
      for (auto &future : c)
        if (future.valid())
          future.wait();
    }
  }

  template <class Index, class Func>
  void pfor(launch policy, Index from, Index to, const Func &f)
  {
    std::array<std::future<void>, PFOR_NTHREADS> futures;

    if (policy == launch::interleaved)
    {
      for (int i = 0; i < PFOR_NTHREADS; ++i)
        futures[i] = std::move(std::async(std::launch::async,
            detail::work_stepped<Index, Func>,
            from, to, PFOR_NTHREADS, std::cref(f)));
    }
    else if (policy == launch::block)
    {
      // Block size is rounded up
      Index block_size = (to - from - 1) / PFOR_NTHREADS + 1;

      for (int i = 0; i < PFOR_NTHREADS; ++i, from += block_size)
        futures[i] = std::move(std::async(std::launch::async,
            detail::work_stepped<Index, Func>,
            from, std::min(to, from + block_size), 1, std::cref(f)));
    }

    detail::join_all(futures);
  }

  template <class Local, class Index,
            class Func, class InitFunc, class ReduceFunc>
  void pfor(launch policy, Index from, Index to, const InitFunc &init,
      const Func &f, const ReduceFunc &reduce = detail::no_op<Local>)
  {
    std::array<std::future<void>, PFOR_NTHREADS> futures;

    if (policy == launch::interleaved)
    {
      for (int i = 0; i < PFOR_NTHREADS; ++i)
        futures[i] = std::move(std::async(std::launch::async,
            detail::work_stepped_local<Index, Func, InitFunc, ReduceFunc>,
            i, from, to, PFOR_NTHREADS,
            std::cref(f), std::cref(init), std::cref(reduce)));
    }
    else if (policy == launch::block)
    {
      // Block size is rounded up
      Index block_size = (to - from - 1) / PFOR_NTHREADS + 1;

      for (int i = 0; i < PFOR_NTHREADS; ++i, from += block_size)
        futures[i] = std::move(std::async(std::launch::async,
            detail::work_stepped_local<Index, Func, InitFunc, ReduceFunc>,
            i, from, std::min(to, from + block_size), 1,
            std::cref(f), std::cref(init), std::cref(reduce)));
    }

    detail::join_all(futures);
  }
}
