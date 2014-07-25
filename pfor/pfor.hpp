#pragma once

#include <thread>
#include <array>
#include <future>
#include <algorithm>
#include <chrono>

#ifndef PFOR_NTHREADS
#define PFOR_NTHREADS (4)
#endif

#define BLOCK_SIZE(x) ((x) + 1) / PFOR_NTHREADS

namespace p
{
  enum class launch
  {
    block,
    interleaved,
  };

  namespace detail
  {
    template <class Index, class Func>
    void work_stepped(Index from, Index to, Index step, const Func &f)
    {
      for (; from < to; from += step) f(from);
    }

    template <class ForwardIterator, class Func>
    void work_stepped_iterator(ForwardIterator from, ForwardIterator to,
        typename ForwardIterator::difference_type step, const Func &f)
    {
      for (; from < to; std::advance(from, step)) f(*from);
    }

    template <class Index, class Func, class InitFunc, class ReduceFunc>
    void work_stepped_local(Index from, Index to, Index step,
        const Func &f, const InitFunc &init, const ReduceFunc &reduce)
    {
      auto local = init();
      for (; from < to; from += step) local = f(from, local);
      reduce(local);
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
      Index block_size = BLOCK_SIZE(to - from);

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
      const Func &f, const ReduceFunc &reduce)
  {
    std::array<std::future<void>, PFOR_NTHREADS> futures;

    if (policy == launch::interleaved)
    {
      for (int i = 0; i < PFOR_NTHREADS; ++i)
        futures[i] = std::move(std::async(std::launch::async,
            detail::work_stepped_local<Index, Func, InitFunc, ReduceFunc>,
            from, to, PFOR_NTHREADS,
            std::cref(f), std::cref(init), std::cref(reduce)));
    }
    else if (policy == launch::block)
    {
      // Block size is rounded up
      Index block_size = BLOCK_SIZE(to - from);

      for (int i = 0; i < PFOR_NTHREADS; ++i, from += block_size)
        futures[i] = std::move(std::async(std::launch::async,
            detail::work_stepped_local<Index, Func, InitFunc, ReduceFunc>,
            from, std::min(to, from + block_size), 1,
            std::cref(f), std::cref(init), std::cref(reduce)));
    }

    detail::join_all(futures);
  }

  template <class ForwardIterator, class Func>
  void pforeach(launch policy, ForwardIterator first, ForwardIterator last,
      const Func &f)
  {
    std::array<std::future<void>, PFOR_NTHREADS> futures;
    typedef typename ForwardIterator::difference_type Index;

    if (policy == launch::interleaved)
    {
      for (int i = 0; i < PFOR_NTHREADS; ++i)
        futures[i] = std::move(std::async(std::launch::async,
            detail::work_stepped_iterator<ForwardIterator, Func>,
            first, last, PFOR_NTHREADS, std::cref(f)));
    }
    else if (policy == launch::block)
    {
      // Block size is rounded up
      Index block_size = BLOCK_SIZE(std::distance(first, last));

      for (int i = 0; i < PFOR_NTHREADS; ++i, first += block_size)
        futures[i] = std::move(std::async(std::launch::async,
            detail::work_stepped_iterator<ForwardIterator, Func>,
            first, std::min(last, first + block_size), 1, std::cref(f)));
    }

    detail::join_all(futures);
  }
}
