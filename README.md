pfor
====

Lightweight parallel for loop in C++11.

Description
-----------

This single file header library provides a lightweight implementation of a parallel for loop designed for multicore machines. It is intended to be easy to use with very little overhead.

Example
-------
```cpp
#include <algorithm>
#include <vector>
#include <atomic>

#include "pfor/pfor.hpp"

int main(void)
{
  std::vector<int> vec;
  std::generate_n(vec.begin(), 1000000, rand);
  
  std::atomic<int> sum(0);
  p::pfor<int>(0ul, vec.size(),
    []() { return 0; },
    [&vec](std::size_t i, int local_sum) { return local_sum + vec[i]; },
    [&sum](int local_sum) { sum += local_sum; });
}
```

Todo
----
- Support for containers and iterators
- Support for optimising small loop bodies
- Support for breaking out of the loop
