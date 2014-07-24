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
    interleaved,
    block,
    dynamic,
  };

  namespace detail
  {
    template <class Index, class Func>
    void work_stepped(Index from, Index to, Index step, const Func &f)
    {
      for (; from < to; from += step)
        f(from);
    }
  }

  template <class Index, class Func>
  void pfor(Index from, Index to, const Func &f, launch policy)
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
    else
    {
      // First launch all the threads
      Index begin = std::min(PFOR_NTHREADS, to - from);
      for (Index i = 0; i < begin; ++from, ++i)
        futures[i] = std::move(std::async(std::launch::async, f, from));

      int i = 0;
      while (from < to)
      {
        if (futures[i].wait_for(std::chrono::milliseconds(PFOR_POLL_MS)) ==
            std::future_status::ready)
        {
          // This thread is ready to do work
          futures[i] = std::move(std::async(std::launch::async, f, from++));
        }

        if (++i >= PFOR_NTHREADS)
          i = 0;
      }
    }

    // Wait for all the threads to finish
    for (int i = 0; i < PFOR_NTHREADS; ++i)
      if (futures[i].valid())
        futures[i].wait();
  }
}
