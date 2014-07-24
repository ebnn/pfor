pfor
====

Lightweight parallel for loop in C++11.

Description
-----------

This single file header library provides a lightweight implementation of a parallel for loop designed for multicore machines. It is intended to be easy to use with very little overhead.

Example
-------
```cpp
#include "pfor/pfor.hpp"
#include <algorithm>

int main(void)
{
  std::vector<int> vec;
  std::generate_n(vec.begin(), 100000, rand);
  
  p::pfor<int>(0, vec.size(), [&](int i, int local_sum) {
    local_sum += vec[i];
  });
}
```

